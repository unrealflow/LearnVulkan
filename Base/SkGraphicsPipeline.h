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
                                const std::vector<VkVertexInputAttributeDescription> *inputAttributes = nullptr);
    

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
    VkDescriptorSet SetupDescriptorSet(std::vector<VkWriteDescriptorSet> &writeSets, bool alloc = true)
    {
        if (alloc)
        {
            VkDescriptorSetAllocateInfo allocInfo = SkInit::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
            VK_CHECK_RESULT(vkAllocateDescriptorSets(appBase->device, &allocInfo, &this->descriptorSet));
        }

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
    void CmdDraw(VkCommandBuffer cmd);
};
