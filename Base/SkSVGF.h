#pragma once
#include "SkBase.h"
#include "SkMemory.h"
class SkSVGF
{
private:
    SkBase *appBase;
    SkMemory *mem;
    std::vector<SkImage *> src;
    std::vector<SkImage> dst;
    std::vector<VkCommandBuffer> taCmds;
    VkImageSubresourceRange subresourceRange;
    VkImageCopy imageCopyRegion;
    SkImage preFrame;
    SkBuffer preVP;
    void BuildCommandBuffers()
    {
        taCmds.resize(appBase->frameBuffers.size());
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = appBase->cmdPool;
        allocateInfo.commandBufferCount = (uint32_t)taCmds.size();
        vkAllocateCommandBuffers(appBase->device, &allocateInfo, taCmds.data());

        imageCopyRegion = {};
        imageCopyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        imageCopyRegion.srcOffset = {0, 0, 0};
        imageCopyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        imageCopyRegion.dstOffset = {0, 0, 0};
        imageCopyRegion.extent = {appBase->width, appBase->height, 1};

        VkBufferCopy bufCopy={};
        bufCopy.size=preVP.size;
        bufCopy.dstOffset=0;
        bufCopy.srcOffset=sizeof(glm::mat4);

        subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        VkCommandBufferBeginInfo cmdBufInfo = SkInit::commandBufferBeginInfo();
        for (size_t i = 0; i < taCmds.size(); i++)
        {
            VK_CHECK_RESULT(vkBeginCommandBuffer(taCmds[i], &cmdBufInfo));
            for (size_t j = 0; j < dst.size(); j++)
            {
                CopyImage(taCmds[i],
                          src[j]->image,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          dst[j].image,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            CopyImage(taCmds[i], appBase->images[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, preFrame.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            vkCmdCopyBuffer(taCmds[i],appBase->vpBuffer.buffer,preVP.buffer,1,&bufCopy);
            VK_CHECK_RESULT(vkEndCommandBuffer(taCmds[i]));
        }
    }
    void CopyImage(VkCommandBuffer cmdBuf, VkImage src, VkImageLayout srcLayout, VkImage dst, VkImageLayout dstLayout)
    {
        SkTools::SetImageLayout(cmdBuf, src,
                                srcLayout,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                subresourceRange);
        SkTools::SetImageLayout(cmdBuf, dst,
                                dstLayout,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                subresourceRange);
        vkCmdCopyImage(cmdBuf,
                       src,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       dst,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &imageCopyRegion);
        SkTools::SetImageLayout(cmdBuf, dst,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                dstLayout,
                                subresourceRange);
        SkTools::SetImageLayout(cmdBuf, src,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                srcLayout,
                                subresourceRange);
    }

public:
    SkSVGF(/* args */) {}
    ~SkSVGF() {}
    void Init(SkBase *initBase, SkMemory *initMem)
    {
        appBase = initBase;
        mem = initMem;
        src.clear();
    }
    uint32_t Register(SkImage *_src)
    {
        this->src.push_back(_src);
        return static_cast<uint32_t>(src.size() - 1);
    }
    void Build()
    {
        dst.resize(src.size());
        mem->CreateSamplerImage(appBase->colorFormat, VK_IMAGE_USAGE_TRANSFER_DST_BIT, &preFrame);
        mem->SetupDescriptor(&preFrame, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        mem->dCreateBuffer(sizeof(glm::mat4) * 2,
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           &preVP);
        mem->SetupDescriptor(&preVP);
        for (size_t i = 0; i < dst.size(); i++)
        {
            mem->CreateSamplerImage(src[i]->format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, &dst[i]);
            mem->SetupDescriptor(&dst[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        BuildCommandBuffers();
    }

    void Submit(uint32_t imageIndex)
    {
        vkWaitForFences(appBase->device, 1, &(appBase->waitFences[imageIndex]), VK_TRUE, UINT64_MAX);
        vkResetFences(appBase->device, 1, &(appBase->waitFences[imageIndex]));
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(taCmds[imageIndex]);
        VK_CHECK_RESULT(vkQueueSubmit(appBase->graphicsQueue, 1, &submitInfo, appBase->waitFences[imageIndex]));
    }
    void CleanUp()
    {
        for (size_t i = 0; i < dst.size(); i++)
        {
            mem->FreeImage(&dst[i]);
        }
        mem->FreeBuffer(&preVP);
        mem->FreeImage(&preFrame);
    }
    VkDescriptorImageInfo *GetDes(uint32_t index)
    {
        if (index >= dst.size())
        {
            return nullptr;
        }
        return &dst[index].descriptor;
    }
    VkDescriptorImageInfo *GetDes()
    {
        return &preFrame.descriptor;
    }
    VkDescriptorBufferInfo *GetVPDes()
    {
        return &preVP.descriptor;
    }
};
