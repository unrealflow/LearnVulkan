# Shader资源绑定

## 顶点、索引数据
1. 在读取模型、加载数据时，确定数据的布局
2. 为数据创建`VkBuffer`，写入数据，设置`VkDescriptorBufferInfo`
3. 设置`VkVertexInputBindingDescription`和`VkVertexInputAttributeDescription`
4. 使用`inputBindings`和`inputAttributes创建VkPipeline`
5. 修改shader，使之与顶点、索引数据相匹配
6. `cmd`绑定顶点、索引缓冲
7. `cmd`绘制

## 描述符
1. 确定要添加的描述符的类型、内存、绑定位置、使用阶段
2. 修改管线的`std::vector<VkDescriptorSetLayoutBinding>`,并创建`VkDescriptorSetLayout`
3. 使用`VkDescriptorSetLayout`创建`VkPipelineLayout`
4. 确定指令池大小，创建`VkDescriptorPool`
5. 使用`VkDescriptorPool`为`VkDescriptorSet`分配空间
6. 设置`VkWriteDescriptorSet`，更新描述符集
7. `cmd`将VkDescriptorSet绑定到相应的`VkPipelineLayout`