#pragma once
#include "SkBase.h"

class SkRenderPass
{
private:
    SkBase *appBase;
    void CreateRenderPass()
    {
        std::array<VkAttachmentDescription, 5> attachments = {};
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

        std::array<VkAttachmentReference, 4> colorReferences;
        colorReferences[0] = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        colorReferences[1] = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        colorReferences[2] = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        colorReferences[3] = {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 4;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 2> subpassDescriptions{};
        subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[0].colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpassDescriptions[0].pColorAttachments = colorReferences.data();
        subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

        VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        std::array<VkAttachmentReference, 3> inputReferences;
        inputReferences[0] = {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputReferences[1] = {2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        inputReferences[2] = {3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[1].colorAttachmentCount = 1;
        subpassDescriptions[1].pColorAttachments = &colorReference;
        subpassDescriptions[1].pDepthStencilAttachment = &depthReference;
        // Use the color attachments filled in the first pass as input attachments
        subpassDescriptions[1].inputAttachmentCount = static_cast<uint32_t>(inputReferences.size());
        subpassDescriptions[1].pInputAttachments = inputReferences.data();

        std::array<VkSubpassDependency, 3> dependencies = {};

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

        dependencies[2].srcSubpass = 0;
        dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

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

    void CreateAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment *attachment)
    {
        VkImageAspectFlags aspectMask = 0;
        VkImageLayout imageLayout;

        attachment->format = format;

        if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        assert(aspectMask > 0);

        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = format;
        imageCI.extent = appBase->getExtent3D();
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

        VK_CHECK_RESULT(vkCreateImage(appBase->device, &imageCI, nullptr, &attachment->image));
        VkMemoryRequirements memReqs{};
        vkGetImageMemoryRequirements(appBase->device, attachment->image, &memReqs);

        VkMemoryAllocateInfo memAllloc{};
        memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllloc.allocationSize = memReqs.size;
        memAllloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(appBase->device, &memAllloc, nullptr, &attachment->memory));
        VK_CHECK_RESULT(vkBindImageMemory(appBase->device, attachment->image, attachment->memory, 0));

        VkImageViewCreateInfo imageViewCI{};
        imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCI.image = attachment->image;
        imageViewCI.format = attachment->format;
        imageViewCI.subresourceRange.baseMipLevel = 0;
        imageViewCI.subresourceRange.levelCount = 1;
        imageViewCI.subresourceRange.baseArrayLayer = 0;
        imageViewCI.subresourceRange.layerCount = 1;
        imageViewCI.subresourceRange.aspectMask = aspectMask;
        // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
        if (attachment->format >= VK_FORMAT_D16_UNORM_S8_UINT)
        {
            imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        VK_CHECK_RESULT(vkCreateImageView(appBase->device, &imageViewCI, nullptr, &attachment->view));
    }
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
