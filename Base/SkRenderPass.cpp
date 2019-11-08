﻿#include "SkRenderPass.h"

void SkRenderPass::CreateRenderPass()
    {
        std::array<VkAttachmentDescription, 7> attachments = {};
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

        attachments[5].format = appBase->post0.format;
        attachments[5].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[5].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[5].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[5].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[5].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[5].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[5].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachments[6].format = appBase->post1.format;
        attachments[6].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[6].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[6].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[6].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[6].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[6].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[6].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        std::array<VkAttachmentReference, 4> colorRef0;
        colorRef0[0] = {6, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        colorRef0[1] = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        colorRef0[2] = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        colorRef0[3] = {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 4;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 4> subpassDescriptions{};
        subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[0].colorAttachmentCount = static_cast<uint32_t>(colorRef0.size());
        subpassDescriptions[0].pColorAttachments = colorRef0.data();
        subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

         std::array<VkAttachmentReference, 1> colorRef1;
         colorRef1[0] = {5, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        //  colorRef1[1] = {5, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        std::array<VkAttachmentReference, 4> inputRef1;
        inputRef1[0] = {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef1[1] = {2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef1[2] = {3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef1[3] = {6, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[1].colorAttachmentCount = static_cast<uint32_t>(colorRef1.size());
        subpassDescriptions[1].pColorAttachments = colorRef1.data();
        subpassDescriptions[1].pDepthStencilAttachment = &depthReference;
        // Use the color attachments filled in the first pass as input attachments
        subpassDescriptions[1].inputAttachmentCount = static_cast<uint32_t>(inputRef1.size());
        subpassDescriptions[1].pInputAttachments = inputRef1.data();

         std::array<VkAttachmentReference, 1> colorRef2;
        //  colorRef2[0] = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
         colorRef2[0] = {6, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        std::array<VkAttachmentReference, 4> inputRef2;
        inputRef2[0] = {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef2[1] = {2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef2[2] = {3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef2[3] = {5, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        subpassDescriptions[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[2].colorAttachmentCount = static_cast<uint32_t>(colorRef2.size());
        subpassDescriptions[2].pColorAttachments = colorRef2.data();
        subpassDescriptions[2].pDepthStencilAttachment = &depthReference;
        // Use the color attachments filled in the first pass as input attachments
        subpassDescriptions[2].inputAttachmentCount = static_cast<uint32_t>(inputRef2.size());
        subpassDescriptions[2].pInputAttachments = inputRef2.data();

        std::array<VkAttachmentReference, 1> colorRef3;
         colorRef3[0] = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        //  colorRef3[1] = {6, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        std::array<VkAttachmentReference, 4> inputRef3;
        inputRef3[0] = {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef3[1] = {2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef3[2] = {3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputRef3[3] = {6, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        subpassDescriptions[3].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[3].colorAttachmentCount = static_cast<uint32_t>(colorRef3.size());
        subpassDescriptions[3].pColorAttachments = colorRef3.data();
        subpassDescriptions[3].pDepthStencilAttachment = &depthReference;
        // Use the color attachments filled in the first pass as input attachments
        subpassDescriptions[3].inputAttachmentCount = static_cast<uint32_t>(inputRef3.size());
        subpassDescriptions[3].pInputAttachments = inputRef3.data();

        std::array<VkSubpassDependency, 5> dependencies = {};

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = 1;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[2].srcSubpass = 1;
        dependencies[2].dstSubpass = 2;
        dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


        dependencies[3].srcSubpass = 2;
        dependencies[3].dstSubpass = 3;
        dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[3].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[3].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[4].srcSubpass = 3;
        dependencies[4].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[4].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[4].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[4].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[4].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[4].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

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
 