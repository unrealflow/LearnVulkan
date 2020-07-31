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
    std::array<VkClearValue, PASS_COUNT+4> clearColors;
    clearColors[0].color = appBase->defaultClearColor;
    clearColors[1].color = clearColor;
    clearColors[2].color = clearColor;
    clearColors[3].color = clearColor;
    clearColors[4].depthStencil = clearDepth;
    for(int i=5;i<PASS_COUNT+4;i++)
    {
        clearColors[i].color = clearColor;
    }
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

VkResult SkCmd::Draw(uint32_t imageIndex)
{

    // vkWaitForFences(appBase->device, 1, &(appBase->waitFences[imageIndex]), VK_TRUE, UINT64_MAX);
    // vkResetFences(appBase->device, 1, &(appBase->waitFences[imageIndex]));
    VkPipelineStageFlags waitMask[2] = {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    std::vector<VkSemaphore> waitSemaphores = {
        appBase->semaphores.rayComplete,
        appBase->semaphores.presentComplete};
    std::vector<VkSemaphore> signalSemaphores = {
        appBase->semaphores.readyForPresent,
        appBase->semaphores.readyForCopy
        };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitMask;
    submitInfo.pSignalSemaphores = signalSemaphores.data();
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(appBase->drawCmdBuffers[imageIndex]);
    // return vkQueueSubmit(appBase->graphicsQueue, 1, &submitInfo, appBase->waitFences[imageIndex]);
    return vkQueueSubmit(appBase->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
}
VkResult SkCmd::Submit(uint32_t imageIndex)
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &(appBase->swapChain);
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &(appBase->semaphores.readyForPresent);
    presentInfo.pImageIndices = &(imageIndex);

    appBase->currentFrame = imageIndex;
    return vkQueuePresentKHR(appBase->presentQueue, &presentInfo);
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