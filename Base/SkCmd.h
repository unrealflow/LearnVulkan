#pragma once
#include "SkBase.h"
#include "SkDevice.h"
#include "SkModel.h"
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
    std::vector<SkModel *> models;

public:
    SkCmd(/* args */);
    ~SkCmd();

    void Init(SkBase *initBase, SkDevice *initDevice)
    {
        fprintf(stderr, "SkCmd::Init...\n");
        appBase = initBase;
        skDevice = initDevice;
        CreateCmdPool();
        CreateSyncObjects();
        models.clear();
    }

    void CreateCmdBuffers()
    {
        appBase->drawCmdBuffers.resize(appBase->frameBuffers.size());
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = appBase->cmdPool;
        allocateInfo.commandBufferCount = (uint32_t)appBase->drawCmdBuffers.size();
        vkAllocateCommandBuffers(appBase->device, &allocateInfo, appBase->drawCmdBuffers.data());

        VkCommandBufferBeginInfo cmdBeginInfo = {};
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = appBase->renderPass;
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = appBase->getExtent();
        VkClearValue clearColor = {0.1f, 0.2f, 0.1f, 1.0f};
        renderPassBeginInfo.pClearValues = &clearColor;
        renderPassBeginInfo.clearValueCount = 1;

        for (size_t i = 0; i < appBase->drawCmdBuffers.size(); i++)
        {

            VK_CHECK_RESULT(vkBeginCommandBuffer(appBase->drawCmdBuffers[i], &cmdBeginInfo));

            renderPassBeginInfo.framebuffer = appBase->frameBuffers[i];
            vkCmdBeginRenderPass(appBase->drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(appBase->drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, appBase->graphicsPipeline);

            for (size_t j = 0; j < models.size(); j++)
            {
                models[j]->CmdDraw(appBase->drawCmdBuffers[i]);
            }

            vkCmdEndRenderPass(appBase->drawCmdBuffers[i]);
            VK_CHECK_RESULT(vkEndCommandBuffer(appBase->drawCmdBuffers[i]));
        }
    }
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
    void Submit()
    {
        uint32_t imageIndex;
        vkAcquireNextImageKHR(appBase->device, appBase->swapChain, UINT64_MAX, appBase->semaphores.presentComplete, (VkFence) nullptr, &(imageIndex));
        vkWaitForFences(appBase->device, 1, &(appBase->waitFences[imageIndex]), VK_TRUE, UINT64_MAX);
        vkResetFences(appBase->device, 1, &(appBase->waitFences[imageIndex]));
        VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &(appBase->semaphores.presentComplete);
        submitInfo.pWaitDstStageMask = &waitMask;
        submitInfo.pSignalSemaphores = &(appBase->semaphores.renderComplete);
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(appBase->drawCmdBuffers[imageIndex]);
        VK_CHECK_RESULT(vkQueueSubmit(appBase->graphicsQueue, 1, &submitInfo, appBase->waitFences[imageIndex]));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &(appBase->swapChain);
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &(appBase->semaphores.renderComplete);
        presentInfo.pImageIndices = &(imageIndex);

        VK_CHECK_RESULT(vkQueuePresentKHR(appBase->presentQueue, &presentInfo));
    }
    void CreateBuffer(
        const void *initData,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer *outBuffer,
        VkDeviceMemory *outMemory)
    {
        skDevice->CreateBuffer(initData, size, usage, outBuffer, outMemory);
    }
    void CreateLocalBuffer(
        const void *initData,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer *outBuffer,
        VkDeviceMemory *outMemory)
    {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;
        skDevice->CreateBuffer(initData, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer, &stagingMemory);
        skDevice->CreateLocalBuffer(size, usage, outBuffer, outMemory);
        VkCommandBuffer copyCmd = getCommandBuffer(true);

        // Put buffer region copies into command buffer
        VkBufferCopy copyRegion = {};

        // Vertex buffer
        copyRegion.size = size;
        vkCmdCopyBuffer(copyCmd, stagingBuffer, *outBuffer, 1, &copyRegion);
        // Index buffer

        // Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
        flushCommandBuffer(copyCmd);
        vkDestroyBuffer(appBase->device, stagingBuffer, nullptr);
        vkFreeMemory(appBase->device, stagingMemory, nullptr);
    }
    VkCommandBuffer getCommandBuffer(bool begin)
    {
        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = appBase->cmdPool;
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(appBase->device, &cmdBufAllocateInfo, &cmdBuffer));

        // If requested, also start the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufInfo = {};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
        }

        return cmdBuffer;
    }

    // End the command buffer and submit it to the queue
    // Uses a fence to ensure command buffer has finished executing before deleting it
    void flushCommandBuffer(VkCommandBuffer commandBuffer)
    {
        assert(commandBuffer != VK_NULL_HANDLE);

        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VkFence fence;
        VK_CHECK_RESULT(vkCreateFence(appBase->device, &fenceCreateInfo, nullptr, &fence));

        // Submit to the queue
        VK_CHECK_RESULT(vkQueueSubmit(appBase->graphicsQueue, 1, &submitInfo, fence));
        // Wait for the fence to signal that command buffer has finished executing
        VK_CHECK_RESULT(vkWaitForFences(appBase->device, 1, &fence, VK_TRUE, UINT32_MAX));

        vkDestroyFence(appBase->device, fence, nullptr);
        vkFreeCommandBuffers(appBase->device, appBase->cmdPool, 1, &commandBuffer);
    }
    void AddModel(SkModel *model, bool useStaging = true)
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
        models.push_back(model);
    }
};

SkCmd::SkCmd(/* args */)
{
}

SkCmd::~SkCmd()
{
}
