﻿#include "SkApp.h"
#include "stdexcept"
#include "iostream"

// #ifdef NDEBUG

// const bool validation = false;
// #else
// const bool validation = true;
// #endif
const bool validation = true;

class SkRender : public SkApp
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Color;
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
    SkModel model;
    SkTexture texture;
    void PrepareVertices()
    {
        model.Init(appBase);
        std::vector<Vertex> verticesData =
            {
                {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            };

        std::vector<uint32_t> indicesData = {0, 1, 2, 0, 2, 3};
        model.LoadVerticesData(verticesData.data(), verticesData.size() * sizeof(Vertex));
        model.LoadIndicesData(indicesData.data(), indicesData.size());
    }
    void PreparePipeline()
    {
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
        inputAttributes[1].offset = offsetof(Vertex, Color);
        inputAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;

        inputAttributes[2].binding = 0;
        inputAttributes[2].location = 2;
        inputAttributes[2].offset = offsetof(Vertex, UV);
        inputAttributes[2].format = VK_FORMAT_R32G32_SFLOAT;

        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)};
        std::vector<VkDescriptorPoolSize> poolSizes = {
            SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1)};
        VkDescriptorPoolCreateInfo descriptorPoolInfo = SkInit::descriptorPoolCreateInfo(poolSizes, 1);
        this->pipeline.SetupLayout(poolSizes, bindings);
        this->pipeline.CreateGraphicsPipeline(&inputBindings, &inputAttributes);
    }
    void PrepareCmd()
    {
        this->cmd.BuildModel(&model);
        texture.Init(appBase, "my.jpg");
        this->cmd.BuildTexture(&texture, true);

        VkDescriptorImageInfo texDescriptor = {};
        texDescriptor.sampler = texture.sampler;
        texDescriptor.imageLayout = texture.imageLayout;
        texDescriptor.imageView = texture.GetImageView();
        
        std::vector<VkWriteDescriptorSet> writeSets = {
            SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texDescriptor)};

        this->pipeline.SetupDescriptorSet(writeSets);
        this->cmd.CreateCmdBuffers();
    }

public:
    SkRender() : SkApp("SkRender", validation)
    {
    }
    void AppSetup() override
    {
        this->pipeline.SetShader("Shader\\vert_3.spv", "Shader\\frag_3.spv");
        PrepareVertices();
        PreparePipeline();
        PrepareCmd();
    }
    void Draw() override
    {
        this->cmd.Submit();
    }
    void CleanUp1() override
    {
        SkApp::CleanUp1();
        model.CleanUp();
        texture.CleanUp();
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