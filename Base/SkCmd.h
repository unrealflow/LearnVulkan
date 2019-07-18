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

public:
    SkCmd(/* args */);

    void Init(SkBase *initBase)
    {
        fprintf(stderr, "SkCmd::Init...\n");
        appBase = initBase;
        CreateCmdPool();
        CreateCmdBuffers();
    }
    void CleanUp()
    {
        vkDestroyCommandPool(appBase->device, appBase->cmdPool, nullptr);
        fprintf(stderr, "SkCmd::CleanUp...\n");
    }

    ~SkCmd();
};

SkCmd::SkCmd(/* args */)
{
}

SkCmd::~SkCmd()
{
}
