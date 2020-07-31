#pragma once
#include "SkBase.h"
#include "SkAgent.h"

class SkRenderPass
{
private:
    SkBase *appBase = nullptr;
    SkAgent *agent = nullptr;
    void CreateRenderPass();

    void CreateFrameBuffers();

    void CleanFrameBuffers()
    {
        fprintf(stderr, "SkRenderPass::CleanFrameBuffers...\n");

        for (size_t i = 0; i < appBase->frameBuffers.size(); i++)
        {
            vkDestroyFramebuffer(appBase->device, appBase->frameBuffers[i], nullptr);
        }
        CleanUpAttachments();
    }
    void CreateAttachments()
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
        agent->CreateAttachment(appBase->depthStencil.format,
                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                &appBase->depthStencil);
        for (int i = 0; i < PASS_COUNT - 1; i++)
        {
            agent->CreateAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                    &appBase->pass[i]);
        }
    }
    void CleanUpAttachments()
    {
        agent->FreeImage(&appBase->position);
        agent->FreeImage(&appBase->normal);
        agent->FreeImage(&appBase->albedo);
        agent->FreeImage(&appBase->depthStencil);
        for (int i = 0; i < PASS_COUNT - 1; i++)
        {
            agent->FreeImage(&appBase->pass[i]);
        }
    }

public:
    void Init(SkBase *initBase, SkAgent *initAgent)
    {
        appBase = initBase;
        agent = initAgent;
        CreateAttachments();
        CreateRenderPass();
        CreateFrameBuffers();
    }
    void RecreateBuffers()
    {
        CleanFrameBuffers();
        CreateAttachments();
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
