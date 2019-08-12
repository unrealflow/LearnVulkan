#pragma once
#include "SkBase.h"
#include "SkMesh.h"
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
    VkDescriptorSet defaultDesSet;
    VkPipelineLayout pipelineLayout;
    std::vector<SkMesh *> meshes;
    VkViewport viewport;
    VkRect2D scissor;
    bool useDynamic;

private:
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
    //useDynamic==true时，管线的viewport和scissor需动态设置
    //initPool不为空时，使用已有的descriptorPool，不在重新创建
    void Init(SkBase *initBase, bool useDynamic = true, VkDescriptorPool initPool = VK_NULL_HANDLE)
    {
        appBase = initBase;
        if (initPool != VK_NULL_HANDLE)
        {
            useExternalPool = true;
        }
        descriptorPool = initPool;
        this->shaderModules.clear();
        meshes.clear();
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
        vertShaderModule = SkTools::CreateShaderModule(appBase->device, vertPath);
        fragShaderModule = SkTools::CreateShaderModule(appBase->device, fragPath);
        this->shaderModules.emplace_back(vertShaderModule);
        this->shaderModules.emplace_back(fragShaderModule);
    }

    void CleanUp()
    {
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
    //设置mesh的descriptorSet，并写入相关管线信息
    //若mesh==nullptr，则写入管线默认的descriptorSet
    void SetupDescriptorSet(SkMesh *mesh, std::vector<VkWriteDescriptorSet> &writeSets, bool alloc = true)
    {
        VkDescriptorSet *target = &defaultDesSet;
        if (mesh != nullptr)
        {
            mesh->pipelineLayout = this->pipelineLayout;
            target = &mesh->desSet;
        }
        if (alloc)
        {
            VkDescriptorSetAllocateInfo allocInfo = SkInit::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
            VK_CHECK_RESULT(vkAllocateDescriptorSets(appBase->device, &allocInfo, target));
        }

        for (size_t i = 0; i < writeSets.size(); i++)
        {
            writeSets[i].dstSet = *target;
        }
        vkUpdateDescriptorSets(appBase->device, (uint32_t)writeSets.size(), writeSets.data(), 0, nullptr);
    }
    void PrepareDynamicState()
    {
        viewport = SkInit::viewport((float)appBase->width, (float)appBase->height, 0.0f, 1.0f);
        scissor = SkInit::rect2D(appBase->width, appBase->height, 0, 0);
    }
    //使用pipeline绘制所记录的mesh
    //当没有mesh需要绘制时，则绘制默认图像
    void CmdDraw(VkCommandBuffer cmd);
};
