#pragma once
#include "SkBase.h"
#include "SkModel.h"
#include "SkTexture.h"
class SkMemory
{
private:
    SkBase *appBase = nullptr;
    const VkMemoryPropertyFlags F_HOST = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    const VkMemoryPropertyFlags F_LOCAL = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkSampler sampler;
public:
    uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
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
    VkCommandBuffer GetCommandBuffer(bool begin, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
    {
        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = appBase->cmdPool;
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(appBase->device, &cmdBufAllocateInfo, &cmdBuffer));

        // If requested, also start the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufInfo = {};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
        }

        return cmdBuffer;
    }

    // End the command buffer and submit it to the queue
    // Uses a fence to ensure command buffer has finished executing before deleting it
    void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true)
    {
        assert(commandBuffer != VK_NULL_HANDLE);

        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VkFence fence;
        VK_CHECK_RESULT(vkCreateFence(appBase->device, &fenceCreateInfo, nullptr, &fence));

        // Submit to the queue
        VK_CHECK_RESULT(vkQueueSubmit(appBase->graphicsQueue, 1, &submitInfo, fence));
        // Wait for the fence to signal that command buffer has finished executing
        VK_CHECK_RESULT(vkWaitForFences(appBase->device, 1, &fence, VK_TRUE, UINT32_MAX));

        vkDestroyFence(appBase->device, fence, nullptr);
        if (free)
        {
            vkFreeCommandBuffers(appBase->device, appBase->cmdPool, 1, &commandBuffer);
        }
    }

public:
    VkDeviceSize dCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mFlags, VkBuffer *outBuffer, VkDeviceMemory *outMemory)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;

        VkMemoryAllocateInfo memAlloc = {};
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

        VkMemoryRequirements memReqs;
        VK_CHECK_RESULT(vkCreateBuffer(appBase->device, &bufferInfo, nullptr, outBuffer));
        vkGetBufferMemoryRequirements(appBase->device, *outBuffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, mFlags);
        VK_CHECK_RESULT(vkAllocateMemory(appBase->device, &memAlloc, nullptr, outMemory));
        VK_CHECK_RESULT(vkBindBufferMemory(appBase->device, *outBuffer, *outMemory, 0));
        return memAlloc.allocationSize;
    }
    VkDeviceSize dCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mFlags, SkBuffer *outBuffer)
    {
        outBuffer->size = size;
        // outBuffer->usage = usage;
        // outBuffer->mFlags = mFlags;
        return dCreateBuffer(size, usage, mFlags, &outBuffer->buffer, &outBuffer->memory);
    }

    VkDeviceSize dCreateImage(VkExtent3D extent,
                              VkImageUsageFlags usage,
                              VkMemoryPropertyFlags mFlags,
                              VkImage *outImage, VkDeviceMemory *outMemory,
                              VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                              VkImageTiling tiling = VK_IMAGE_TILING_LINEAR)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageInfo.extent = extent;
        VK_CHECK_RESULT(vkCreateImage(appBase->device, &imageInfo, nullptr, outImage));

        VkMemoryRequirements memReqs = {};
        vkGetImageMemoryRequirements(appBase->device, *outImage, &memReqs);

        VkMemoryAllocateInfo memAlloc = {};
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, mFlags);
        VK_CHECK_RESULT(vkAllocateMemory(appBase->device, &memAlloc, nullptr, outMemory));
        VK_CHECK_RESULT(vkBindImageMemory(appBase->device, *outImage, *outMemory, 0));
        return memReqs.size;
    }

public:
    SkMemory(/* args */) {}
    ~SkMemory() {}
    void Init(SkBase *initBase)
    {
        appBase = initBase;
        CreateSampler(&sampler);
    }
    void CleanUp()
    {
        vkDestroySampler(appBase->device,sampler,nullptr);
    }
    VkDeviceSize WriteMemory(VkDeviceMemory dst, const void *src, VkDeviceSize size)
    {
        void *data;
        VK_CHECK_RESULT(vkMapMemory(appBase->device, dst, 0, size, 0, &data));
        memcpy(data, src, size);
        vkUnmapMemory(appBase->device, dst);
        return size;
    }
    void *Map(VkDeviceMemory dst, VkDeviceSize size)
    {
        void *data;
        VK_CHECK_RESULT(vkMapMemory(appBase->device, dst, 0, size, 0, &data));
        return data;
    }
    void *Map(SkBuffer *buf)
    {
        VK_CHECK_RESULT(vkMapMemory(appBase->device, buf->memory, 0, buf->size, 0, &buf->data));
        return buf->data;
    }
    void Unmap(VkDeviceMemory dst)
    {
        vkUnmapMemory(appBase->device, dst);
    }
    void Unmap(SkBuffer *buf)
    {
        vkUnmapMemory(appBase->device, buf->memory);
        buf->data = nullptr;
    }
    void CreateBuffer(const void *initData,
                      VkDeviceSize size,
                      VkBufferUsageFlags usage,
                      VkBuffer *outBuffer,
                      VkDeviceMemory *outMemory)
    {
        this->dCreateBuffer(size, usage, F_HOST, outBuffer, outMemory);
        WriteMemory(*outMemory, initData, size);
    }
    void CreateBuffer(const void *initData,
                      VkDeviceSize size,
                      VkBufferUsageFlags usage,
                      SkBuffer *outBuffer)
    {
        this->dCreateBuffer(size, usage, F_HOST, outBuffer);
        WriteMemory(outBuffer->memory, initData, size);
    }
    void CreateLocalBuffer(const void *initData,
                           VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkBuffer *outBuffer,
                           VkDeviceMemory *outMemory)
    {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;
        this->dCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, F_HOST, &stagingBuffer, &stagingMemory);
        this->WriteMemory(stagingMemory, initData, size);
        this->dCreateBuffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, F_LOCAL, outBuffer, outMemory);
        VkCommandBuffer copyCmd = GetCommandBuffer(true);

        VkBufferCopy copyRegion = {};

        copyRegion.size = size;
        vkCmdCopyBuffer(copyCmd, stagingBuffer, *outBuffer, 1, &copyRegion);

        FlushCommandBuffer(copyCmd);
        vkDestroyBuffer(appBase->device, stagingBuffer, nullptr);
        vkFreeMemory(appBase->device, stagingMemory, nullptr);
    }
    void CreateImage(const void *initData,
                     VkExtent3D extent,
                     VkImageUsageFlags usage,
                     VkImage *outImage,
                     VkDeviceMemory *outMemory,
                     VkImageLayout *layout)
    {

        VkDeviceSize size = this->dCreateImage(extent, usage, F_HOST, outImage, outMemory);
        this->WriteMemory(*outMemory, initData, size);
        VkCommandBuffer copyCmd = this->GetCommandBuffer(true);

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;

        VkImageMemoryBarrier imageMemoryBarrier = SkInit::imageMemoryBarrier();
        imageMemoryBarrier.image = *outImage;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
        *layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        this->FlushCommandBuffer(copyCmd);
    }
    void CreateLocalImage(const void *initData,
                          VkExtent3D extent,
                          VkImageUsageFlags usage,
                          VkImage *outImage,
                          VkDeviceMemory *outMemory,
                          VkImageLayout *layout)
    {

        this->dCreateImage(extent, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT, F_LOCAL, outImage, outMemory);
        VkDeviceSize size = SkTools::CalSize(extent) * sizeof(unsigned char) * 4;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;
        this->dCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, F_HOST, &stagingBuffer, &stagingMemory);
        this->WriteMemory(stagingMemory, initData, size);
        VkCommandBuffer copyCmd = GetCommandBuffer(true);
        VkBufferImageCopy copyRegion = {};
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = extent;
        copyRegion.bufferOffset = 0;

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;

        VkImageMemoryBarrier imageMemoryBarrier = SkInit::imageMemoryBarrier();
        imageMemoryBarrier.image = *outImage;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
        vkCmdCopyBufferToImage(
            copyCmd,
            stagingBuffer,
            *outImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &copyRegion);
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
        *layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        this->FlushCommandBuffer(copyCmd, true);

        vkFreeMemory(appBase->device, stagingMemory, nullptr);
        vkDestroyBuffer(appBase->device, stagingBuffer, nullptr);
    }
    void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageView *outView)
    {
        VkImageViewCreateInfo imageViewCI{};
        imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCI.image = image;
        imageViewCI.format = format;
        imageViewCI.subresourceRange.baseMipLevel = 0;
        imageViewCI.subresourceRange.levelCount = 1;
        imageViewCI.subresourceRange.baseArrayLayer = 0;
        imageViewCI.subresourceRange.layerCount = 1;
        imageViewCI.subresourceRange.aspectMask = aspectMask;
        VK_CHECK_RESULT(vkCreateImageView(appBase->device, &imageViewCI, nullptr, outView));
    }
    void BuildModel(SkModel *model, bool useStaging = true)
    {

        if (useStaging)
        {
            CreateLocalBuffer(model->verticesData.data(), model->GetVertexBufferSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &model->vertices.buffer, &model->vertices.memory);
            CreateLocalBuffer(model->indicesData.data(), model->GetIndexBufferSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT|VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &model->indices.buffer, &model->indices.memory);
        }
        else
        {
            CreateBuffer(model->verticesData.data(), model->GetVertexBufferSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &model->vertices.buffer, &model->vertices.memory);
            CreateBuffer(model->indicesData.data(), model->GetIndexBufferSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &model->indices.buffer, &model->indices.memory);
        }
    }
    void BuildTexture(SkTexture *tex, bool useStaging = false)
    {
        if (useStaging)
        {
            this->CreateLocalImage(tex->data, tex->GetExtent3D(), VK_IMAGE_USAGE_SAMPLED_BIT, &tex->image, &tex->deviceMemory, &tex->imageLayout);
        }
        else
        {
            this->CreateImage(tex->data, tex->GetExtent3D(), VK_IMAGE_USAGE_SAMPLED_BIT, &tex->image, &tex->deviceMemory, &tex->imageLayout);
        }
        CreateImageView(tex->image, tex->format, VK_IMAGE_ASPECT_COLOR_BIT, &tex->view);
    }
    void CreateAttachment(VkFormat format, VkImageUsageFlags usage, SkImage *attachment)
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
        this->dCreateImage(appBase->getExtent3D(),
                           usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, F_LOCAL,
                           &attachment->image,
                           &attachment->memory,
                           format,
                           VK_IMAGE_TILING_OPTIMAL);

        if (attachment->format >= VK_FORMAT_D16_UNORM_S8_UINT)
        {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        CreateImageView(attachment->image, attachment->format, aspectMask, &attachment->view);
    }
    void CreateStorageImage(SkImage *out, bool fetchable)
    {
        out->format = appBase->colorFormat;
        VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT;

        usage |= fetchable ? VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        this->dCreateImage(appBase->getExtent3D(),
                           usage,
                           F_LOCAL, &out->image, &out->memory,
                           out->format, VK_IMAGE_TILING_OPTIMAL);
        this->CreateImageView(out->image, out->format, VK_IMAGE_ASPECT_COLOR_BIT, &out->view);
        VkCommandBuffer cmdBuf = GetCommandBuffer(true);
        SkTools::SetImageLayout(cmdBuf, out->image,
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                                {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
        FlushCommandBuffer(cmdBuf);
    }
    void CreateSamplerImage(VkFormat format, VkImageUsageFlags usage,SkImage *out)
    {
        out->format = format;
        
        usage = VK_IMAGE_USAGE_SAMPLED_BIT|usage;

        this->dCreateImage(appBase->getExtent3D(),
                           usage,
                           F_LOCAL, &out->image, &out->memory,
                           out->format, VK_IMAGE_TILING_OPTIMAL);
        this->CreateImageView(out->image, out->format, VK_IMAGE_ASPECT_COLOR_BIT, &out->view);
        VkCommandBuffer cmdBuf = GetCommandBuffer(true);
        SkTools::SetImageLayout(cmdBuf, out->image,
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
        FlushCommandBuffer(cmdBuf);
    }
    void CreateSampler(VkSampler *outSampler)
    {
        VkSamplerCreateInfo samplerCI = SkInit::samplerCreateInfo();
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.minFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.mipLodBias = 0.0f;
        samplerCI.compareOp = VK_COMPARE_OP_NEVER;
        samplerCI.minLod = 0.0f;
        samplerCI.maxLod = 0.0f;
        samplerCI.maxAnisotropy = 1.0;
        samplerCI.anisotropyEnable = VK_FALSE;
        samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        VK_CHECK_RESULT(vkCreateSampler(appBase->device, &samplerCI, nullptr, outSampler));
    }
    void SetupDescriptor(SkBuffer *buf)
    {
        buf->descriptor = {};
        buf->descriptor.buffer = buf->buffer;
        buf->descriptor.range = buf->size;
    }
    void SetupDescriptor(SkImage *img,VkImageLayout layout)
    {
        img->descriptor = {};
        img->descriptor.imageLayout=layout;
        img->descriptor.imageView=img->view;
        img->descriptor.sampler=sampler;
    }
    void SetupDescriptor(SkImage *img,VkImageLayout layout,VkSampler sampler)
    {
        img->descriptor = {};
        img->descriptor.imageLayout=layout;
        img->descriptor.imageView=img->view;
        img->descriptor.sampler=sampler;
    }
    inline void FreeImage(SkImage *sImage)
    {
        vkDestroyImageView(appBase->device, sImage->view, nullptr);
        vkFreeMemory(appBase->device, sImage->memory, nullptr);
        vkDestroyImage(appBase->device, sImage->image, nullptr);

        sImage->view = VK_NULL_HANDLE;
        sImage->memory = VK_NULL_HANDLE;
        sImage->image = VK_NULL_HANDLE;
    }
    inline void FreeBuffer(VkBuffer *buffer, VkDeviceMemory *memory)
    {
        vkFreeMemory(appBase->device, *memory, nullptr);
        vkDestroyBuffer(appBase->device, *buffer, nullptr);
        *memory = VK_NULL_HANDLE;
        *buffer = VK_NULL_HANDLE;
    }
    inline void FreeBuffer(SkBuffer *buf)
    {
        vkFreeMemory(appBase->device, buf->memory, nullptr);
        vkDestroyBuffer(appBase->device, buf->buffer, nullptr);
        buf->memory = VK_NULL_HANDLE;
        buf->buffer = VK_NULL_HANDLE;
        buf->data = nullptr;
    }
    inline void FreeShaderModules(std::vector<VkShaderModule> &shaderModules)
    {
        for (auto &&i : shaderModules)
        {
            vkDestroyShaderModule(appBase->device, i, nullptr);
        }
        shaderModules.clear();
    }
    inline void FreeLayout(VkPipelineLayout *pLayout)
    {
        vkDestroyPipelineLayout(appBase->device, *pLayout, nullptr);
        *pLayout = VK_NULL_HANDLE;
    }
    inline void FreeLayout(VkDescriptorSetLayout *pLayout)
    {
        vkDestroyDescriptorSetLayout(appBase->device, *pLayout, nullptr);
        *pLayout = VK_NULL_HANDLE;
    }
    inline void FreePipeline(VkPipeline *pipeline)
    {
        vkDestroyPipeline(appBase->device, *pipeline, nullptr);
        *pipeline = VK_NULL_HANDLE;
    }
    inline void FreeDescriptorPool(VkDescriptorPool *DescriptorPool)
    {
        vkDestroyDescriptorPool(appBase->device, *DescriptorPool, nullptr);
        *DescriptorPool = VK_NULL_HANDLE;
    }
};
