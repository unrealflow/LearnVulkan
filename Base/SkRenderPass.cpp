#include "SkRenderPass.h"

void SkRenderPass::CreateRenderPass()
{
    std::array<VkAttachmentDescription, 4 + PASS_COUNT> attachments = {};
    attachments[0].format = appBase->colorFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Deferred attachments
    // Position
    attachments[1].format = appBase->position.format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // Normals
    attachments[2].format = appBase->normal.format;
    attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // Albedo
    attachments[3].format = appBase->albedo.format;
    attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // Depth attachment
    attachments[4].format = appBase->depthStencil.format;
    attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    for (int i = 0; i < PASS_COUNT - 1; i++)
    {
        int t = 5 + i;
        attachments[t].format = appBase->pass[i].format;
        attachments[t].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[t].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[t].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[t].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[t].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[t].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[t].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    std::array<VkSubpassDescription, PASS_COUNT+1> subpassDescriptions{};
    std::array<VkAttachmentReference, 3> colorRef0;
    colorRef0[0] = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    colorRef0[1] = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    colorRef0[2] = {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 4;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[0].colorAttachmentCount = static_cast<uint32_t>(colorRef0.size());
    subpassDescriptions[0].pColorAttachments = colorRef0.data();
    subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

    std::array<VkAttachmentReference, PASS_COUNT> colorRef;
    std::array<std::vector<VkAttachmentReference>,PASS_COUNT> inputRef;
    for (uint32_t i = 1; i < PASS_COUNT + 1; i++)
    {
        int p=i-1;
        colorRef[p] = {i == PASS_COUNT ? 0 : 4 + i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        inputRef[p].resize(3);
        inputRef[p][0] = {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef[p][1] = {2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef[p][2] = {3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        if (i > 1)
        {
            inputRef[p].emplace_back(VkAttachmentReference{3 + i, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        }

        subpassDescriptions[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // subpassDescriptions[i].colorAttachmentCount = static_cast<uint32_t>(colorRef.size());
        // subpassDescriptions[i].pColorAttachments = colorRef.data();
        subpassDescriptions[i].colorAttachmentCount = 1;
        subpassDescriptions[i].pColorAttachments = &colorRef[p];
        // subpassDescriptions[i].pDepthStencilAttachment = &depthReference;
        subpassDescriptions[i].pDepthStencilAttachment = NULL;
        // Use the color attachments filled in the first pass as input attachments
        subpassDescriptions[i].inputAttachmentCount = static_cast<uint32_t>(inputRef[p].size());
        subpassDescriptions[i].pInputAttachments = inputRef[p].data();
    }

    std::array<VkSubpassDependency, PASS_COUNT + 2> dependencies = {};

    {
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        int t = PASS_COUNT + 1;
        dependencies[t].srcSubpass = t - 1;
        dependencies[t].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[t].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[t].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[t].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[t].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[t].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }
    for (int i = 1; i < PASS_COUNT + 1; i++)
    {
        dependencies[i].srcSubpass = i - 1;
        dependencies[i].dstSubpass = i;
        dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(appBase->device, &renderPassInfo, nullptr, &(appBase->renderPass)));
}

void SkRenderPass::CreateFrameBuffers()
{
    std::array<VkImageView, 4 + PASS_COUNT> attachments;
    // VkImageView attachments[2];
    attachments[1] = appBase->position.view;
    attachments[2] = appBase->normal.view;
    attachments[3] = appBase->albedo.view;
    attachments[4] = appBase->depthStencil.view;
    for (int i = 0; i < PASS_COUNT - 1; i++)
    {
        attachments[5 + i] = appBase->pass[i].view;
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = appBase->renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = appBase->width;
    framebufferInfo.height = appBase->height;
    framebufferInfo.layers = 1;

    appBase->frameBuffers.resize(static_cast<size_t>(appBase->imageCount));

    for (size_t i = 0; i < appBase->frameBuffers.size(); i++)
    {
        attachments[0] = appBase->imageViews[i];
        VK_CHECK_RESULT(vkCreateFramebuffer(appBase->device, &framebufferInfo, nullptr, &appBase->frameBuffers[i]));
    }
}