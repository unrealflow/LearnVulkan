#include "SkCmd.h"

void SkCmd::CreateCmdBuffers()
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
    VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
    VkClearDepthStencilValue clearDepth = {1.0f, 0};
    std::array<VkClearValue, 5> clearColors;
    clearColors[0].color = appBase->defaultClearColor;
    clearColors[1].color = clearColor;
    clearColors[2].color = clearColor;
    clearColors[3].color = clearColor;
    clearColors[4].depthStencil = clearDepth;
    renderPassBeginInfo.pClearValues = clearColors.data();
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());

    for (size_t i = 0; i < appBase->drawCmdBuffers.size(); i++)
    {
        size_t sub = 0;
        VK_CHECK_RESULT(vkBeginCommandBuffer(appBase->drawCmdBuffers[i], &cmdBeginInfo));

        renderPassBeginInfo.framebuffer = appBase->frameBuffers[i];
        vkCmdBeginRenderPass(appBase->drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        while (sub < pipelines.size())
        {
            for (size_t j = 0; j < pipelines[sub].size(); j++)
            {
                pipelines[sub][j]->CmdDraw(appBase->drawCmdBuffers[i]);
            }
            if (sub < pipelines.size() - 1)
            {
                vkCmdNextSubpass(appBase->drawCmdBuffers[i], VK_SUBPASS_CONTENTS_INLINE);
            }
            sub++;
        }
        vkCmdEndRenderPass(appBase->drawCmdBuffers[i]);
        VK_CHECK_RESULT(vkEndCommandBuffer(appBase->drawCmdBuffers[i]));
    }
}

VkResult SkCmd::Submit()
{
    uint32_t imageIndex;
    VkResult result = (vkAcquireNextImageKHR(appBase->device, appBase->swapChain, UINT64_MAX, appBase->semaphores.presentComplete, (VkFence) nullptr, &(imageIndex)));
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        vkResetFences(appBase->device, 1, &(appBase->waitFences[appBase->currentFrame]));
        return result;
    }
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

    appBase->currentFrame = imageIndex;
    return (vkQueuePresentKHR(appBase->presentQueue, &presentInfo));
}

void SkCmd::CreateLocalBuffer(const void *initData,
                              VkDeviceSize size,
                              VkBufferUsageFlags usage,
                              VkBuffer *outBuffer,
                              VkDeviceMemory *outMemory)
{
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    skDevice->CreateBuffer(initData, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer, &stagingMemory);
    skDevice->CreateLocalBuffer(size, usage, outBuffer, outMemory);
    VkCommandBuffer copyCmd = GetCommandBuffer(true);

    // Put buffer region copies into command buffer
    VkBufferCopy copyRegion = {};

    // Vertex buffer
    copyRegion.size = size;
    vkCmdCopyBuffer(copyCmd, stagingBuffer, *outBuffer, 1, &copyRegion);
    // Index buffer

    // Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
    FlushCommandBuffer(copyCmd);
    vkDestroyBuffer(appBase->device, stagingBuffer, nullptr);
    vkFreeMemory(appBase->device, stagingMemory, nullptr);
}
void SkCmd::CreateImage(const void *initData,
                        VkExtent3D extent,
                        VkImageUsageFlags usage,
                        VkImage *outImage,
                        VkDeviceMemory *outMemory,
                        VkImageLayout *layout)
{

    skDevice->CreateImage(initData, extent, usage, outImage, outMemory);
    VkCommandBuffer copyCmd = this->GetCommandBuffer(true);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    VkImageMemoryBarrier imageMemoryBarrier = SkInit::imageMemoryBarrier();
    imageMemoryBarrier.image = *outImage;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkCmdPipelineBarrier(
        copyCmd,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
    *layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    this->FlushCommandBuffer(copyCmd);
}
void SkCmd::CreateLocalImage(const void *initData,
                             VkExtent3D extent,
                             VkImageUsageFlags usage,
                             VkImage *outImage,
                             VkDeviceMemory *outMemory,
                             VkImageLayout *layout)
{

    skDevice->CreateLocalImage(extent, usage, outImage, outMemory);
    VkDeviceSize size = SkTools::CalSize(extent) * sizeof(unsigned char) * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    skDevice->CreateBuffer(initData, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer, &stagingMemory);

    VkCommandBuffer copyCmd = GetCommandBuffer(true);
    VkBufferImageCopy copyRegion = {};
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageExtent = extent;
    copyRegion.bufferOffset = 0;

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    VkImageMemoryBarrier imageMemoryBarrier = SkInit::imageMemoryBarrier();
    imageMemoryBarrier.image = *outImage;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    vkCmdPipelineBarrier(
        copyCmd,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
    vkCmdCopyBufferToImage(
        copyCmd,
        stagingBuffer,
        *outImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &copyRegion);
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCmdPipelineBarrier(
        copyCmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
    *layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    this->FlushCommandBuffer(copyCmd, true);

    vkFreeMemory(appBase->device, stagingMemory, nullptr);
    vkDestroyBuffer(appBase->device, stagingBuffer, nullptr);
}
VkCommandBuffer SkCmd::GetCommandBuffer(bool begin)
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
    void SkCmd::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
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
        if (free)
        {
            vkFreeCommandBuffers(appBase->device, appBase->cmdPool, 1, &commandBuffer);
        }
    }