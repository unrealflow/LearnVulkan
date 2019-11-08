#pragma once
#include "SkBase.h"
#include "SkGraphicsPipeline.h"

//创建指令池，创建和提交渲染相关指令
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

    void CreateSyncObjects()
    {

        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreateSemaphore(appBase->device, &semaphoreCreateInfo, nullptr, &(appBase->semaphores.presentComplete)));
        VK_CHECK_RESULT(vkCreateSemaphore(appBase->device, &semaphoreCreateInfo, nullptr, &(appBase->semaphores.readyForPresent)));
        VK_CHECK_RESULT(vkCreateSemaphore(appBase->device, &semaphoreCreateInfo, nullptr, &(appBase->semaphores.readyForCopy)));
        VK_CHECK_RESULT(vkCreateSemaphore(appBase->device, &semaphoreCreateInfo, nullptr, &(appBase->semaphores.rayComplete)));

        appBase->waitFences.resize(appBase->imageCount);
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < appBase->waitFences.size(); i++)
        {
            VK_CHECK_RESULT(vkCreateFence(appBase->device, &fenceCreateInfo, nullptr, &(appBase->waitFences[i])));
        }
    }

    //记录各subpass所使用的pipeline
    std::vector<std::vector<SkGraphicsPipeline *>> pipelines;

public:
    SkCmd(/* args */) {}
    ~SkCmd() {}

    void Init(SkBase *initBase)
    {
        appBase = initBase;
        pipelines.clear();
        CreateCmdPool();
        CreateSyncObjects();
    }

    //记录指定的subpass使用的pipeline
    void RegisterPipeline(SkGraphicsPipeline *pipeline, uint32_t subpass)
    {
        while (pipelines.size() <= subpass)
        {
            std::vector<SkGraphicsPipeline *> temp = {};
            pipelines.push_back(temp);
        }
        pipelines[subpass].push_back(pipeline);
    }
    //创建每一帧需要提交的指令缓冲
    void CreateCmdBuffers();
    void CleanUp()
    {
        vkDestroySemaphore(appBase->device, appBase->semaphores.rayComplete, nullptr);
        vkDestroySemaphore(appBase->device, appBase->semaphores.presentComplete, nullptr);
        vkDestroySemaphore(appBase->device, appBase->semaphores.readyForPresent, nullptr);
        vkDestroySemaphore(appBase->device, appBase->semaphores.readyForCopy, nullptr);
        for (size_t i = 0; i < appBase->waitFences.size(); i++)
        {
            vkDestroyFence(appBase->device, appBase->waitFences[i], nullptr);
        }

        vkDestroyCommandPool(appBase->device, appBase->cmdPool, nullptr);
    }

    //根据imageIndex提交对应的指令缓冲，并将处理后的图像提交到呈现队列
    //在调用前需调用CreateCmdBuffers()创建指令缓冲
    VkResult Draw(uint32_t imageIndex);
    VkResult Submit(uint32_t imageIndex);

    //开始一个指令
    //begin为true时会自动调用vkBeginCommandBuffer(...)
    VkCommandBuffer GetCommandBuffer(bool begin);

    //执行一个指令，并等待指令执行完毕
    //free为true时会将指令销毁
    void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);

    void FreeCmdBuffers()
    {
        fprintf(stderr, "SkCmd::RecreateCmdBuffers...\n");
        vkFreeCommandBuffers(appBase->device, appBase->cmdPool, static_cast<uint32_t>(appBase->drawCmdBuffers.size()), appBase->drawCmdBuffers.data());
    }
};
