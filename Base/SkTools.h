#pragma once

#include "SkCommon.h"
#include <math.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <fstream>

#define VK_CHECK_RESULT(f)                                                                                                                        \
    {                                                                                                                                             \
        VkResult _res_ = (f);                                                                                                                     \
        if (_res_ != VK_SUCCESS)                                                                                                                  \
        {                                                                                                                                         \
            std::cout << "Fatal : VkResult is \"" << SkTools::ErrorString(_res_) << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
            assert(_res_ == VK_SUCCESS);                                                                                                          \
        }                                                                                                                                         \
    }
#define SK_SHOW(paramter) fprintf(stderr, "%s :%d...\n", #paramter, paramter);
static inline std::string DataDir()
{
#if defined(VK_EXAMPLE_DATA_DIR)
    return VK_EXAMPLE_DATA_DIR;
#else
    return "";
#endif
}
namespace SkTools
{

std::string ErrorString(VkResult errorCode);
static std::vector<char> readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        fprintf(stderr,"Failed to open %s...\n",filename.c_str());
        
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
static inline VkDeviceSize CalSize(VkExtent3D extent)
{
    return extent.width * extent.height * extent.depth;
}
void SetImageLayout(
    VkCommandBuffer cmdbuffer,
    VkImage image,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
void SetImageLayout(
    VkCommandBuffer cmdbuffer,
    VkImage image,
    VkImageAspectFlags aspectMask,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char> &code);

VkShaderModule CreateShaderModule(VkDevice device, const std::string path);

} // namespace SkTools
