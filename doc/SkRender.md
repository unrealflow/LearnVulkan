# SkRender

## 绘制流程
1. 确定要使用的扩展和校验层，创建`VkInstance`
    * 基础扩展：
        * `glfwGetRequiredInstanceExtensions()`
        * `VK_EXT_debug_utils`
    * 光追扩展：
        * `VK_KHR_get_physical_device_properties`

2. 确定需要启用的功能及扩展，创建VkDevice
    * 基础扩展：
        * `VK_KHR_swapchain`
    * 光追扩展：
        * `VK_KHR_get_memory_requirements2`
        * `VK_NV_ray_tracing`

3. 创建`VkSwapchainKHR`，获取交换链中图像，为交换链图像创建视图

4. 创建GBuffer
    * 创建4个`VkImage`，作为GBuffer储存位置、法线、颜色、深度。

5.  设置并创建`VkRenderPass`
    * 为RenderPass设置7个Attachment，Attachment要与关联的图像相匹配，其中：
        * `attachments[0]`表示交换链中的图像
        * `attachments[1]`表示GBuffer中的位置
        * `attachments[2]`表示GBuffer中的法线
        * `attachments[3]`表示GBuffer中的颜色
        * `attachments[4]`表示GBuffer中的深度，作为深度缓冲,用于开启深度测试
        * `attachments[5]`和`attachments[6]`用于自定义的后处理阶段的缓冲

    * 设置4个subpass，其中：
        * `subpass[0]`为GBuffer流程，将`attachments[1~3、6]`作为输出，用于渲染模型，生成GBuffer，同时对光追结果图像进行预处理。
        * `subpass[1]`为降噪流程，将`attachments[1~3、6]`作为输入，`attachment[5]`作为输出，仅渲染窗口矩形，结合GBuffer中的信息，对图像进行降噪。
        * `subpass[2]`和`subpass[3]`为后处理流程，用于实现伽马矫正、泛光等后处理效果。
    
    * 为每个subpass设置`VkSubpassDependency`，确保上一个subpass完成写入图像后才开始下一个subpass的读取

6. 创建帧缓冲
    * 与RenderPass的attachments对应，为帧缓冲设置7个Attachment，并写入相关图像视图

7. 创建`VkCommandPool`，创建用于同步的`VkSemaphore`和`VkFence`
8. 设置相机，设置鼠标、键盘等事件调用。
9. 导入模型，设置材质，设置灯光等
10. 加载shader，创建四个`VkPipeline`，分别设置管线的输入描述、管线布局等，其中；
    * `pipeline[0]`用于渲染模型，生成GBuffer
    * `pipeline[1]`用于读取GBuffer，渲染一个用于输出到窗口的矩形
    * `pipeline[2]`和 `pipeline[3]`用于添加后处理效果。

11. 为顶点、索引、相机、材质、灯光等数据创建`VkBuffer`、`VkDeviceMemory`，将数据写入显存
12. 进行光线追踪流程的初始化。
13. 准备其他资源，如创建用于储存上一帧GBuffer、绘制结果等的图像缓冲。
14. 创建`VkDescriptorPool`，使用描述符池分配`VkDescriptorSet`
15. 为要写入描述符集的数据设置`VkWriteDescriptorSet`，更新描述符集
16. 创建用于每帧提交的`VkCommandBuffer`，记录的指令大概为：
    1. 开始指令缓冲
    2. 开始RenderPass，自动进入`subpass[0]`
        1. 绑定管线`pipeline[0]`
        2. 设置视图
        3. 绑定管线对应的描述符
        4. 绑定顶点、索引缓冲，绘制
    3. 遍历后续的subpass，在每个subpass中：
        1. 绑定管线
        2. 设置视图
        3. 绑定管线对应的描述符
        4. 绘制
        5. 进入下一个subpass
    4. 结束RenderPass
    5. 结束指令缓冲
17. 记录其他后处理指令，如将GBuffer、绘制结果复制到图像缓冲。

18. 进入渲染循环，在循环中执行以下调用：
    1. 更新相机等信息对应的`VkBuffer`和`VkDeviceMemory`
    2. 提交光线追踪指令
    3. 提交绘制指令
    4. 提交后处理指令。

19. 等待循环退出，进行资源的释放和清理

## 光线追踪流程
1. 查询设备对扩展的支持信息，使用`vkGetDeviceProcAddr`函数来获取扩展函数的地址
2. 加载模型数据
    1. 使用三个容器分别收集所有网格的顶点、索引、材质信息，注意索引须加入顶点的偏移信息
    2. 创建缓冲，将容器内数据加载至显存，设置描述符
    3. 创建`VkGeometryNV`
    4. 使用`VkGeometryNV`，创建底层加速结构
    5. 添加实例信息，创建顶层加速结构
    6. 使用`vkCmdBuildAccelerationStructureNV`建立加速结构

3. 创建用于保存结果的图像，准备需要写入的缓冲
4. 在`pipeline[1]`的布局添加一个图像采样器绑定，将图像视图写入`pipeline[1]`的描述符集
5, 加载shader，创建光线追踪管线
    1. 在**Ray Generation Shader**中调用`traceNV()`发出光线，根据光线的返回信息将颜色写入图像
    2. 在**Closest Hit Shader**中，使用内建变量的值计算位置、法线等信息，进行光照计算
    3. 在**Miss Shade**r中，处理未命中任何物体的光线，如设置背景值等
    4. `traceNV()`根据传入的参数来选择要使用的shader，每个shader中均可多次调用`traceNV()`
    5. 可添加更多自定义的shader，使用不同方式调用`traceNV()`，实现阴影、折射等效果

6. 创建`ShaderBindingTable`
7. 创建并写入描述符集
8. 记录光线追踪指令
9. 在渲染循环中：
    1.更新描述符集
    2.在绘制指令之前提交指令，生成光追效果图像，供绘制流程使用。
10. 程序退出时进行资源的释放和清理

#本程序仅实现了一个简单的光照模型，更多关于光照计算的知识请参考：

* [浅墨的游戏编程 - 毛星云](https://zhuanlan.zhihu.com/game-programming)


