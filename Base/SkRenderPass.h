#pragma once
#include "SkBase.h"
#include "SkAgent.h"

class SkRenderPass
{
private:
    SkBase *appBase = nullptr;
    SkAgent *agent = nullptr;
    void CreateRenderPass();

    void CreateFrameBuffers()
    {
        std::array<VkImageView, 7> attachments;
        // VkImageView attachments[2];
        attachments[1] = appBase->position.view;
        attachments[2] = appBase->normal.view;
        attachments[3] = appBase->albedo.view;
        attachments[4] = appBase->depthStencil.view;
        attachments[5] = appBase->post0.view;
        attachments[6] = appBase->post1.view;

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
        agent->CreateAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              &appBase->position); // (World space) Positions
        agent->CreateAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              &appBase->normal); // (World space) Normals
        agent->CreateAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              &appBase->albedo);
        agent->CreateAttachment(appBase->depthStencil.format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &appBase->depthStencil);
        agent->CreateAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              &appBase->post0);
        agent->CreateAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              &appBase->post1);
    }
    void CleanUpGBufferAttachments()
    {
        agent->FreeImage(&appBase->position);
        agent->FreeImage(&appBase->normal);
        agent->FreeImage(&appBase->albedo);
        agent->FreeImage(&appBase->depthStencil);
        agent->FreeImage(&appBase->post0);
        agent->FreeImage(&appBase->post1);
    }

public:
    void Init(SkBase *initBase, SkAgent *initAgent)
    {
        appBase = initBase;
        agent = initAgent;
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

        CleanFrameBuffers();
        vkDestroyRenderPass(appBase->device, appBase->renderPass, nullptr);
    }
    ~SkRenderPass()
    {
    }
};
