#include "SkApp.h"
#include "stdexcept"
#include "iostream"

#ifdef NDEBUG

const bool validation = false;
#else
const bool validation = true;
#endif
// const bool validation = true;

class SkRender : public SkApp
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 UV;
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
    // SkModel model;
    // SkTexture texture;
    SkScene scene;
    SkTexture *pTexture;
    void PrepareVertices()
    {
        // model.Init(appBase);
        // std::vector<Vertex> verticesData =
        //     {
        //         {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        //         {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        //         {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        //         {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        //     };

        // std::vector<uint32_t> indicesData = {0, 1, 2, 0, 2, 3};
        // model.LoadVerticesData(verticesData.data(), verticesData.size() * sizeof(Vertex));
        // model.LoadIndicesData(indicesData.data(), indicesData.size());
        scene.Init(appBase);
        scene.ImportModel("Plane.dae");
        fprintf(stderr, "%zd,%zd...\n", scene.model.verticesData.size(), scene.model.indicesData.size());
        fprintf(stderr, "%d,%d...\n", scene.vertexCount, scene.indexCount);
    }
    void PreparePipeline()
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {
            SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
            SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1),
            SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 4),
            };
        this->pipeline.CreateDescriptorPool(poolSizes);

        std::vector<VkDescriptorSetLayoutBinding> bindings;

        bindings.clear();
        bindings = {
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
        };

        this->pipeline.SetShader("Shader\\vert_3_denoise.spv", "Shader\\frag_3_denoise.spv");
        this->pipeline.CreateDescriptorSetLayout(bindings);
        appBase->denoisePipeline = this->pipeline.CreateGraphicsPipeline(1, 1);

        std::vector<VkVertexInputBindingDescription> inputBindings;
        std::vector<VkVertexInputAttributeDescription> inputAttributes;

        inputBindings.resize(1);
        inputBindings[0].binding = 0;
        inputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindings[0].stride = sizeof(Vertex);
        inputAttributes.resize(3);
        inputAttributes[0].binding = 0;
        inputAttributes[0].location = 0;
        inputAttributes[0].offset = offsetof(Vertex, Position);
        inputAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;

        inputAttributes[1].binding = 0;
        inputAttributes[1].location = 1;
        inputAttributes[1].offset = offsetof(Vertex, Normal);
        inputAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;

        inputAttributes[2].binding = 0;
        inputAttributes[2].location = 2;
        inputAttributes[2].offset = offsetof(Vertex, UV);
        inputAttributes[2].format = VK_FORMAT_R32G32_SFLOAT;

        bindings.clear();
        bindings = {
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)};

        this->pipeline.SetShader("Shader\\vert_3_gbuffer.spv", "Shader\\frag_3_gbuffer.spv");
        this->pipeline.CreateDescriptorSetLayout(bindings);
        appBase->gBufferPipeline = this->pipeline.CreateGraphicsPipeline(0, 4, &inputBindings, &inputAttributes);

        
    }
    void PrepareCmd()
    {
        // this->cmd.BuildModel(&scene.model);
        // // // texture.Init(appBase, "my.jpg");
        // pTexture=scene.textures[0].id;
        // this->cmd.BuildTexture(pTexture, true);
        scene.Build(&cmd);
        VkDescriptorImageInfo texDescriptor = scene.GetTexDescriptor(0);
        VkDescriptorBufferInfo bufDescriptor = callback.GetCamDescriptor();
        std::vector<VkWriteDescriptorSet> writeSets = {
            SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufDescriptor),
            SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptor)};

        this->pipeline.SetupDescriptorSet(writeSets);
        this->cmd.CreateCmdBuffers();
    }

public:
    SkRender() : SkApp("SkRender", validation)
    {
    }
    void AppSetup() override
    {
        SkApp::AppSetup();
        PrepareVertices();
        PreparePipeline();
        PrepareCmd();
    }
    void CleanUp1() override
    {
        SkApp::CleanUp1();
        // scene.model.CleanUp();
        // pTexture->CleanUp();
        scene.CleanUp();
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