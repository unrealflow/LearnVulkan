#pragma once
#include "SkBase.h"
#include "SkModel.h"
#include "SkTexture.h"

class SkGraphicsPipeline
{
private:
    /* data */
    SkBase *appBase;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
    //记录Shader模块，便于重用和清理
    std::vector<VkShaderModule> shaderModules;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    bool useExternalPool = false;

public:
    //描述符池
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    std::vector<SkModel *> models;
    VkViewport viewport;
    VkRect2D scissor;
    bool useDynamic;

private:
    VkShaderModule createShaderModule(const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(appBase->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }
    void SetInput(
        const std::vector<VkVertexInputBindingDescription> *inputBindings = nullptr,
        const std::vector<VkVertexInputAttributeDescription> *inputAttributes = nullptr)
    {
        vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        if (inputBindings)
        {
            vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(inputBindings->size());
            vertexInputInfo.pVertexBindingDescriptions = inputBindings->data();
        }
        if (inputAttributes)
        {
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributes->size());
            vertexInputInfo.pVertexAttributeDescriptions = inputAttributes->data();
        }
    }

public:
    void Init(SkBase *initBase, bool useDynamic = true, VkDescriptorPool initPool = VK_NULL_HANDLE)
    {
        appBase = initBase;
        if (initPool != VK_NULL_HANDLE)
        {
            useExternalPool = true;
        }
        descriptorPool = initPool;
        this->shaderModules.clear();
        models.clear();
        fprintf(stderr, "SkGraphicsPipeline::Init...\n");
        this->useDynamic = useDynamic;
        if (useDynamic)
        {
            PrepareDynamicState();
        }
    }

    //在调用之前需先设置Shader和Input
    void CreateGraphicsPipeline(uint32_t subpass, uint32_t attachCount,
                                const std::vector<VkVertexInputBindingDescription> *inputBindings = nullptr,
                                const std::vector<VkVertexInputAttributeDescription> *inputAttributes = nullptr)
    {
        fprintf(stderr, "Create Pipeline...\n");

        SetInput(inputBindings, inputAttributes);
        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)appBase->width;
        viewport.height = (float)appBase->height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = appBase->getExtent();

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments = {};
        colorBlendAttachments.resize(attachCount);
        colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachments[0].blendEnable = VK_FALSE;
        for (size_t i = 1; i < colorBlendAttachments.size(); i++)
        {
            colorBlendAttachments[i] = colorBlendAttachments[0];
        }

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.depthTestEnable = true;
        depthStencilStateCreateInfo.depthWriteEnable = true;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilStateCreateInfo.front = depthStencilStateCreateInfo.back;
        depthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
        colorBlending.pAttachments = colorBlendAttachments.data();
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;
        VkPipelineDynamicStateCreateInfo dynamicState={};
        std::vector<VkDynamicState> dynamicStateEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};
        if (useDynamic)
        {
            dynamicState = SkInit::pipelineDynamicStateCreateInfo(
                dynamicStateEnables.data(),
                static_cast<uint32_t>(dynamicStateEnables.size()));
        }
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        pipelineInfo.layout = this->pipelineLayout;
        pipelineInfo.renderPass = appBase->renderPass;
        pipelineInfo.subpass = subpass;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState = useDynamic? &dynamicState:nullptr;
        VK_CHECK_RESULT(vkCreateGraphicsPipelines(appBase->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
    }

    void SetShader(const std::string vertPath, const std::string fragPath)
    {
        auto vertShaderCode = SkTools::readFile(vertPath);
        auto fragShaderCode = SkTools::readFile(fragPath);
        vertShaderModule = createShaderModule(vertShaderCode);
        fragShaderModule = createShaderModule(fragShaderCode);
        this->shaderModules.emplace_back(vertShaderModule);
        this->shaderModules.emplace_back(fragShaderModule);
    }

    void CleanUp()
    {
        fprintf(stderr, "SkGraphicsPipeline::CleanUp...\n");
        for (size_t i = 0; i < this->shaderModules.size(); i++)
        {
            vkDestroyShaderModule(appBase->device, this->shaderModules[i], nullptr);
        }
        vkDestroyDescriptorSetLayout(appBase->device, this->descriptorSetLayout, nullptr);
        vkDestroyPipelineLayout(appBase->device, this->pipelineLayout, nullptr);
        if (!useExternalPool)
        {
            vkDestroyDescriptorPool(appBase->device, descriptorPool, nullptr);
        }
        vkDestroyPipeline(appBase->device, this->pipeline, nullptr);
    }
    void CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &bindings)
    {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = SkInit::descriptorSetLayoutCreateInfo(bindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(appBase->device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = SkInit::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
        VK_CHECK_RESULT(vkCreatePipelineLayout(appBase->device, &pipelineLayoutCreateInfo, nullptr, &this->pipelineLayout));
    }

    VkDescriptorPool CreateDescriptorPool(const std::vector<VkDescriptorPoolSize> &poolSizes, uint32_t maxSets)
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo = SkInit::descriptorPoolCreateInfo(poolSizes, maxSets);
        VK_CHECK_RESULT(vkCreateDescriptorPool(appBase->device, &descriptorPoolInfo, nullptr, &descriptorPool));
        return descriptorPool;
    }

    void SetupBlankLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {};
        this->CreateDescriptorSetLayout(bindings);
    }
    VkDescriptorSet SetupDescriptorSet(std::vector<VkWriteDescriptorSet> &writeSets)
    {
        VkDescriptorSetAllocateInfo allocInfo = SkInit::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
        VK_CHECK_RESULT(vkAllocateDescriptorSets(appBase->device, &allocInfo, &this->descriptorSet));
        for (size_t i = 0; i < writeSets.size(); i++)
        {
            writeSets[i].dstSet = this->descriptorSet;
        }
        vkUpdateDescriptorSets(appBase->device, (uint32_t)writeSets.size(), writeSets.data(), 0, nullptr);
        return this->descriptorSet;
    }
    void PrepareDynamicState()
    {
        viewport = SkInit::viewport((float)appBase->width, (float)appBase->height, 0.0f, 1.0f);
        scissor = SkInit::rect2D(appBase->width, appBase->height, 0, 0);
    }
    void CmdDraw(VkCommandBuffer cmd)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1, &this->descriptorSet, 0, nullptr);
        if (useDynamic)
        {
            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetScissor(cmd, 0, 1, &scissor);
        }
        if (models.empty())
        {
            vkCmdDraw(cmd, 3, 1, 0, 0);
            return;
        }
        for (size_t j = 0; j < models.size(); j++)
        {
            models[j]->CmdDraw(cmd);
        }
    }
};
