#pragma once
#include "SkBase.h"
#include "SkDevice.h"
#include "SkGraphicsPipeline.h"
class SkCmd
{
private:
    SkBase *appBase;
    SkDevice *skDevice;

    void CreateCmdPool()
    {
        VkCommandPoolCreateInfo cmdCreateInfo = {};
        cmdCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdCreateInfo.queueFamilyIndex = appBase->familyIndices.graphicsFamily.value();
        VK_CHECK_RESULT(vkCreateCommandPool(appBase->device, &cmdCreateInfo, nullptr, &(appBase->cmdPool)));
    }

    void CreateSyncObjects()
    {

        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreateSemaphore(appBase->device, &semaphoreCreateInfo, nullptr, &(appBase->semaphores.presentComplete)));
        VK_CHECK_RESULT(vkCreateSemaphore(appBase->device, &semaphoreCreateInfo, nullptr, &(appBase->semaphores.renderComplete)));

        appBase->waitFences.resize(appBase->imageCount);
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < appBase->waitFences.size(); i++)
        {
            VK_CHECK_RESULT(vkCreateFence(appBase->device, &fenceCreateInfo, nullptr, &(appBase->waitFences[i])));
        }
    }
    std::vector<std::vector<SkGraphicsPipeline *>> pipelines;

public:
    SkCmd(/* args */) {}
    ~SkCmd() {}

    void Init(SkBase *initBase, SkDevice *initDevice)
    {
        fprintf(stderr, "SkCmd::Init...\n");
        appBase = initBase;
        skDevice = initDevice;
        pipelines.clear();
        CreateCmdPool();
        CreateSyncObjects();
    }
    void RegisterPipeline(SkGraphicsPipeline *pipeline, uint32_t subpass)
    {
        while (pipelines.size() <= subpass)
        {
            std::vector<SkGraphicsPipeline *> temp = {};
            pipelines.push_back(temp);
        }
        pipelines[subpass].push_back(pipeline);
    }
    void CreateCmdBuffers();
    void CleanUp()
    {
        vkDestroySemaphore(appBase->device, appBase->semaphores.presentComplete, nullptr);
        vkDestroySemaphore(appBase->device, appBase->semaphores.renderComplete, nullptr);
        for (size_t i = 0; i < appBase->waitFences.size(); i++)
        {
            vkDestroyFence(appBase->device, appBase->waitFences[i], nullptr);
        }

        vkDestroyCommandPool(appBase->device, appBase->cmdPool, nullptr);
        fprintf(stderr, "SkCmd::CleanUp...\n");
    }
    VkResult Submit();
    void CreateBuffer(const void *initData,
                      VkDeviceSize size,
                      VkBufferUsageFlags usage,
                      VkBuffer *outBuffer,
                      VkDeviceMemory *outMemory)
    {
        skDevice->CreateBuffer(initData, size, usage, outBuffer, outMemory);
    }
    void CreateLocalBuffer(const void *initData,
                           VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkBuffer *outBuffer,
                           VkDeviceMemory *outMemory);

    void CreateImage(const void *initData,
                     VkExtent3D extent,
                     VkImageUsageFlags usage,
                     VkImage *outImage,
                     VkDeviceMemory *outMemory,
                     VkImageLayout *layout);

    void CreateLocalImage(const void *initData,
                          VkExtent3D extent,
                          VkImageUsageFlags usage,
                          VkImage *outImage,
                          VkDeviceMemory *outMemory,
                          VkImageLayout *layout);

    VkCommandBuffer GetCommandBuffer(bool begin);

    // End the command buffer and submit it to the queue
    // Uses a fence to ensure command buffer has finished executing before deleting it
    void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
    void BuildModel(SkModel *model, bool useStaging = true)
    {

        if (useStaging)
        {
            CreateLocalBuffer(model->verticesData.data(), model->GetVertexBufferSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &model->vertices.buffer, &model->vertices.memory);
            CreateLocalBuffer(model->indicesData.data(), model->GetIndexBufferSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &model->indices.buffer, &model->indices.memory);
        }
        else
        {
            CreateBuffer(model->verticesData.data(), model->GetVertexBufferSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &model->vertices.buffer, &model->vertices.memory);
            CreateBuffer(model->indicesData.data(), model->GetIndexBufferSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &model->indices.buffer, &model->indices.memory);
        }
    }
    void BuildTexture(SkTexture *tex, bool useStaging = false)
    {
        if (useStaging)
        {
            this->CreateLocalImage(tex->data, tex->GetExtent3D(), VK_IMAGE_USAGE_SAMPLED_BIT, &tex->image, &tex->deviceMemory, &tex->imageLayout);
        }
        else
        {
            this->CreateImage(tex->data, tex->GetExtent3D(), VK_IMAGE_USAGE_SAMPLED_BIT, &tex->image, &tex->deviceMemory, &tex->imageLayout);
        }
    }
    void FreeCmdBuffers()
    {
        fprintf(stderr, "SkCmd::RecreateCmdBuffers...\n");
        vkFreeCommandBuffers(appBase->device, appBase->cmdPool, static_cast<uint32_t>(appBase->drawCmdBuffers.size()), appBase->drawCmdBuffers.data());
    }
};
