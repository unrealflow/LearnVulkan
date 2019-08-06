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

    void BuildCommandBuffers()
    {
        taCmds.resize(appBase->frameBuffers.size());
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = appBase->cmdPool;
        allocateInfo.commandBufferCount = (uint32_t)taCmds.size();
        vkAllocateCommandBuffers(appBase->device, &allocateInfo, taCmds.data());

        VkImageCopy copyRegion{};
        copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copyRegion.srcOffset = {0, 0, 0};
        copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copyRegion.dstOffset = {0, 0, 0};
        copyRegion.extent = {appBase->width, appBase->height, 1};

        VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        VkCommandBufferBeginInfo cmdBufInfo = SkInit::commandBufferBeginInfo();
        for (size_t i = 0; i < taCmds.size(); i++)
        {
            VK_CHECK_RESULT(vkBeginCommandBuffer(taCmds[i], &cmdBufInfo));
            for (size_t j = 0; j < dst.size(); j++)
            {

                SkTools::SetImageLayout(taCmds[i], src[j]->image,
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        subresourceRange);
                SkTools::SetImageLayout(taCmds[i], dst[j].image,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        subresourceRange);
                vkCmdCopyImage(taCmds[i],
                               src[j]->image,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               dst[j].image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1, &copyRegion);
                SkTools::SetImageLayout(taCmds[i], dst[j].image,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        subresourceRange);
                SkTools::SetImageLayout(taCmds[i], src[j]->image,
                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        subresourceRange);
            }

            VK_CHECK_RESULT(vkEndCommandBuffer(taCmds[i]));
        }
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
    }
    VkDescriptorImageInfo* GetDes(uint32_t index)
    {
        if(index>=dst.size())
        {
            return nullptr;
        }
        return &dst[index].descriptor;
    }
};
