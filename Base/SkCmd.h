#pragma once
#include "SkBase.h"
class SkCmd
{
private:
    SkBase *appBase;

    void CreateCmdPool()
    {
        VkCommandPoolCreateInfo cmdCreateInfo = {};
        cmdCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdCreateInfo.queueFamilyIndex = appBase->familyIndices.graphicsFamily.value();
        VK_CHECK_RESULT(vkCreateCommandPool(appBase->device, &cmdCreateInfo, nullptr, &(appBase->cmdPool)));
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

            vkCmdDraw(appBase->drawCmdBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(appBase->drawCmdBuffers[i]);
            VK_CHECK_RESULT(vkEndCommandBuffer(appBase->drawCmdBuffers[i]));
        }
    }
    void CreateSyncObjects()
    {

        VkSemaphoreCreateInfo semaphoreCreateInfo={};
        semaphoreCreateInfo.sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreateSemaphore(appBase->device,&semaphoreCreateInfo,nullptr,&(appBase->semaphores.presentComplete)));
        VK_CHECK_RESULT(vkCreateSemaphore(appBase->device,&semaphoreCreateInfo,nullptr,&(appBase->semaphores.renderComplete)));

        appBase->waitFences.resize(appBase->imageCount);
        VkFenceCreateInfo fenceCreateInfo={};
        fenceCreateInfo.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags=VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < appBase->waitFences.size(); i++)
        {
            VK_CHECK_RESULT(vkCreateFence(appBase->device,&fenceCreateInfo,nullptr,&(appBase->waitFences[i])));
        }
        
    }
    

public:
    SkCmd(/* args */);

    void Init(SkBase *initBase)
    {
        fprintf(stderr, "SkCmd::Init...\n");
        appBase = initBase;
        CreateCmdPool();
        CreateCmdBuffers();
        CreateSyncObjects();
    }
    void CleanUp()
    {
        vkDestroySemaphore(appBase->device,appBase->semaphores.presentComplete,nullptr);
        vkDestroySemaphore(appBase->device,appBase->semaphores.renderComplete,nullptr);
        for (size_t i = 0; i < appBase->waitFences.size(); i++)
        {
            vkDestroyFence(appBase->device,appBase->waitFences[i],nullptr);
        }
        
        vkDestroyCommandPool(appBase->device, appBase->cmdPool, nullptr);
        fprintf(stderr, "SkCmd::CleanUp...\n");
    }
    void Submit()
    {
        uint32_t imageIndex;
        vkAcquireNextImageKHR(appBase->device,appBase->swapChain,UINT64_MAX,appBase->semaphores.presentComplete,(VkFence)nullptr,&(imageIndex));
        vkWaitForFences(appBase->device,1,&(appBase->waitFences[imageIndex]),VK_TRUE,UINT64_MAX);
        vkResetFences(appBase->device,1,&(appBase->waitFences[imageIndex]));
        VkPipelineStageFlags waitMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo={};
        submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount=1;
        submitInfo.pWaitSemaphores=&(appBase->semaphores.presentComplete);
        submitInfo.pWaitDstStageMask=&waitMask;
        submitInfo.pSignalSemaphores=&(appBase->semaphores.renderComplete);
        submitInfo.signalSemaphoreCount=1;
        submitInfo.commandBufferCount=1;
        submitInfo.pCommandBuffers=&(appBase->drawCmdBuffers[imageIndex]);
        VK_CHECK_RESULT(vkQueueSubmit(appBase->graphicsQueue,1,&submitInfo,appBase->waitFences[imageIndex]));

        VkPresentInfoKHR presentInfo={};
        presentInfo.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.swapchainCount=1;
        presentInfo.pSwapchains=&(appBase->swapChain);
        presentInfo.waitSemaphoreCount=1;
        presentInfo.pWaitSemaphores=&(appBase->semaphores.renderComplete);
        presentInfo.pImageIndices=&(imageIndex);
        
        VK_CHECK_RESULT(vkQueuePresentKHR(appBase->presentQueue,&presentInfo));
    }
    ~SkCmd();
};

SkCmd::SkCmd(/* args */)
{
}

SkCmd::~SkCmd()
{
}
