#pragma once
#include "SkBase.h"

class SkRenderPass
{
private:
    SkBase *appBase;
    void CreateRenderPass();
    
    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
    {
        // Iterate over all memory types available for the device used in this example
        for (uint32_t i = 0; i < appBase->deviceMemoryProperties.memoryTypeCount; i++)
        {
            if ((typeBits & 1) == 1)
            {
                if ((appBase->deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }
            typeBits >>= 1;
        }
        throw "Could not find a suitable memory type!";
    }

    void CreateAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment *attachment);

    void CreateFrameBuffers()
    {
        std::array<VkImageView, 5> attachments;
        // VkImageView attachments[2];
        attachments[1] = appBase->position.view;
        attachments[2] = appBase->normal.view;
        attachments[3] = appBase->albedo.view;
        attachments[4] = appBase->depthStencil.view;

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = appBase->renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = appBase->width;
        framebufferInfo.height = appBase->height;
        framebufferInfo.layers = 1;

        appBase->frameBuffers.resize(appBase->imageCount);
        for (size_t i = 0; i < appBase->frameBuffers.size(); i++)
        {
            attachments[0] = appBase->imageViews[i];
            VK_CHECK_RESULT(vkCreateFramebuffer(appBase->device, &framebufferInfo, nullptr, &appBase->frameBuffers[i]));
        }
    }
    void CleanFrameBuffers()
    {
        fprintf(stderr, "SkRenderPass::CleanFrameBuffers...\n");

        for (size_t i = 0; i < appBase->frameBuffers.size(); i++)
        {
            vkDestroyFramebuffer(appBase->device, appBase->frameBuffers[i], nullptr);
        }
        vkDestroyImageView(appBase->device, appBase->depthStencil.view, nullptr);
        vkFreeMemory(appBase->device, appBase->depthStencil.memory, nullptr);
        vkDestroyImage(appBase->device, appBase->depthStencil.image, nullptr);
        CleanUpGBufferAttachments();
    }
    void CreateGBufferAttachments()
    {
        CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &appBase->position); // (World space) Positions
        CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &appBase->normal);   // (World space) Normals
        CreateAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &appBase->albedo);
    }
    void CleanUpGBufferAttachments()
    {
        vkDestroyImageView(appBase->device, appBase->position.view, nullptr);
        vkFreeMemory(appBase->device, appBase->position.memory, nullptr);
        vkDestroyImage(appBase->device, appBase->position.image, nullptr);

        vkDestroyImageView(appBase->device, appBase->normal.view, nullptr);
        vkFreeMemory(appBase->device, appBase->normal.memory, nullptr);
        vkDestroyImage(appBase->device, appBase->normal.image, nullptr);

        vkDestroyImageView(appBase->device, appBase->albedo.view, nullptr);
        vkFreeMemory(appBase->device, appBase->albedo.memory, nullptr);
        vkDestroyImage(appBase->device, appBase->albedo.image, nullptr);
    }

public:
    void Init(SkBase *initBase)
    {
        fprintf(stderr, "SkRenderPass::Init...\n");
        appBase = initBase;
        CreateAttachment(appBase->depthStencil.format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &appBase->depthStencil);
        CreateGBufferAttachments();
        CreateRenderPass();
        CreateFrameBuffers();
    }
    void RecreateBuffers()
    {
        CleanFrameBuffers();
        CreateAttachment(appBase->depthStencil.format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &appBase->depthStencil);
        CreateGBufferAttachments();
        CreateFrameBuffers();
    }
    void CleanUp()
    {
        fprintf(stderr, "SkRenderPass::CleanUp...\n");

        CleanFrameBuffers();
        vkDestroyRenderPass(appBase->device, appBase->renderPass, nullptr);
    }
    ~SkRenderPass()
    {
    }
};
