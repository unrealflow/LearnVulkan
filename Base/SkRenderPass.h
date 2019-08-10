#pragma once
#include "SkBase.h"
#include "SkMemory.h"

class SkRenderPass
{
private:
    SkBase *appBase = nullptr;
    SkMemory *mem = nullptr;
    void CreateRenderPass();

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
        CleanUpGBufferAttachments();
    }
    void CreateGBufferAttachments()
    {
        mem->CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              &appBase->position); // (World space) Positions
        mem->CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              &appBase->normal); // (World space) Normals
        mem->CreateAttachment(VK_FORMAT_R8G8B8A8_UNORM,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              &appBase->albedo);
        mem->CreateAttachment(appBase->depthStencil.format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &appBase->depthStencil);
    }
    void CleanUpGBufferAttachments()
    {
        mem->FreeImage(&appBase->position);
        mem->FreeImage(&appBase->normal);
        mem->FreeImage(&appBase->albedo);
        mem->FreeImage(&appBase->depthStencil);
    }

public:
    void Init(SkBase *initBase, SkMemory *initMem)
    {
        fprintf(stderr, "SkRenderPass::Init...\n");
        appBase = initBase;
        mem = initMem;
        CreateGBufferAttachments();
        CreateRenderPass();
        CreateFrameBuffers();
    }
    void RecreateBuffers()
    {
        CleanFrameBuffers();
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
