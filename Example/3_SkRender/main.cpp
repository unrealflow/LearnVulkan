#include "SkApp.h"
#include "stdexcept"
#include "iostream"

#ifdef NDEBUG

const bool validation = false;
#else
const bool validation = true;
#endif

class SkRender : public SkApp
{
    struct Vertex
    {
        float Position[3];
        float Color[3];
    };
    struct
    {
        VkDeviceMemory memory; // Handle to the device memory for this buffer
        VkBuffer buffer;       // Handle to the Vulkan buffer object that the memory is bound to
    } vertices;

    // Index buffer
    struct
    {
        VkDeviceMemory memory;
        VkBuffer buffer;
        uint32_t count;
    } indices;

    void PrepareVertices()
    {
        std::vector<Vertex> verticesData =
            {
                {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            };

        std::vector<uint32_t> indicesData = {0, 1, 2, 0, 2, 3};

        uint32_t vertexBufferSize = static_cast<uint32_t>(verticesData.size()) * sizeof(Vertex);
        indices.count = static_cast<uint32_t>(indicesData.size());
        uint32_t indexBufferSize = indices.count * sizeof(uint32_t);

        this->cmd.CreateLocalBuffer(verticesData.data(),vertexBufferSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,&vertices.buffer,&vertices.memory);
        this->cmd.CreateLocalBuffer(indicesData.data(),indexBufferSize,VK_BUFFER_USAGE_INDEX_BUFFER_BIT,&indices.buffer,&indices.memory);
    }
    void PrepareInputDescription()
    {
        std::vector<VkVertexInputBindingDescription> inputBindings;
        std::vector<VkVertexInputAttributeDescription> inputAttributes;

        inputBindings.resize(1);
        inputBindings[0].binding = 0;
        inputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindings[0].stride = sizeof(Vertex);
        inputAttributes.resize(2);
        inputAttributes[0].binding = 0;
        inputAttributes[0].location = 0;
        inputAttributes[0].offset = offsetof(Vertex, Position);
        inputAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;

        inputAttributes[1].binding = 0;
        inputAttributes[1].location = 1;
        inputAttributes[1].offset = offsetof(Vertex, Color);
        inputAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        this->pipeline.CreateGraphicsPipeline(&inputBindings, &inputAttributes);
    }

public:
    SkRender() : SkApp("SkRender", validation)
    {
    }
    void AppSetup() override
    {
        this->pipeline.SetShader("Shader\\vert_3.spv", "Shader\\frag_3.spv");

        PrepareInputDescription();
        PrepareVertices();

        this->cmd.SetDrawIndexed(vertices.buffer, indices.buffer, indices.count);
        this->cmd.CreateCmdBuffers();
    }
    void Draw() override
    {
        this->cmd.Submit();
    }
    void CleanUp1() override
    {
        vkFreeMemory(appBase->device, indices.memory, nullptr);
        vkDestroyBuffer(appBase->device, indices.buffer, nullptr);
        vkFreeMemory(appBase->device, vertices.memory, nullptr);
        vkDestroyBuffer(appBase->device, vertices.buffer, nullptr);
    }
};

int main()
{
    SkRender skApp;
    try
    {
        skApp.Run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}