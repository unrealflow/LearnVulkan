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

## 内存与显存的对齐问题
1. shader中结构体会被划分为多个`vec4`来储存，例如：
    ```glsl    
    //glsl
    struct P
    {
        float a;
        float b;
        float c;
        float d;
        float e;
    }
    ```
    该结构体在显存中使用两个`vec4`：
    * `vec4 ( a , b , c , d )`
    * `vec4 ( e , * , * , * )`

2. 结构体中，`vec2`、`vec3`等变量不会被拆分到两个vec4中，若剩余空间不够则会再使用一个`vec4`来包含，例如：
    ```glsl
    //glsl
    struct P
    {
        float a;
        float b;
        vec3  c;
        vec3  d;
    }
    ```
    该结构体会使用4个`vec4`：
    * `vec4 ( a , b , * , * )`
    * `vec4 ( c , * )`
    * `vec4 ( d , *)`
    将结构体重新排列一下，变为：
    ```glsl
    //glsl
    struct P
    {
        float a;
        vec3  c;
        float b;
        vec3  d;
    }
    ```
    该结构体仅使用2个`vec4`：
    * `vec4 ( a , c )`
    * `vec4 ( b , d )`

3. 因此，对于从内存传至显存的结构体数据，应将其按每4个`float`的大小排列，以使内存和显存中数据对齐。也可在shader中使用`vec4`数组来接受数据，通过计算解包，例如；
    ```cpp
    //cpp
    struct P
    {
        float a[3];
        float b[3];
        float c[2];
    }
    ```
    ```glsl
     //glsl
    struct Q
    {
        vec3 a;
        vec3 b;
        vec2 c;
    }
    ```
    对于c结构体P和glsl中结构体Q，我们可以采取两种方法保持对齐：
    1. 加入填充数据：
        * 在cpp中，修改结构体P为：
            ```cpp
            //cpp
            struct P
            {
                float a[3];
                float _PAD0_;
                float b[3];
                float _PAD1_;            
                float c[2];
                float _PAD2_;
                float _PAD3_;
            }
            ```
            其中，`_PAD0_`等用于占用内存空间，使其与显存中布局一致

    2. 使用两个vec4接受数据，手动解包数据：
        * 保持结构体P、Q不变，在shader中使用结构体R来接收数据，并解包成Q：
            ```glsl
            //glsl
            struct R
            {
                vec4 a;
                vec4 b;
            }
            Q Unpack(R r)
            {
                Q q;
                q.a = r.a.xyz;
                q.b = vec3(r.a.w , r.b.x , r.b.y);
                q.c = r.b.zw;
                return q;
            }
            ```
            
