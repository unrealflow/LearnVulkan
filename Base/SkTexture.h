#pragma once
#include "SkBase.h"

class SkTexture
{
private:
    SkBase *appBase;
    VkImageView view = VK_NULL_HANDLE;

public:
    unsigned char *data;
    VkSampler sampler;
    VkImage image = VK_NULL_HANDLE;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
    VkDescriptorSet descriptorSet;
    uint32_t width, height;
    int nrChannels;
    // uint32_t mipLevels;
    VkFormat format;

    SkTexture(/* args */);
    ~SkTexture();
    void Init(SkBase *initBase, const char *Path)
    {
        appBase = initBase;
        int _width, _height;
        data = stbi_load(Path, &_width, &_height, &nrChannels, 4);
        width = static_cast<uint32_t>(_width);
        height = static_cast<uint32_t>(_height);
        format = VK_FORMAT_R8G8B8A8_UNORM;
        // assert(format != VK_FORMAT_UNDEFINED);
        fprintf(stderr, "channels:%d...\n", nrChannels);

        if (data)
        {
            CreateSampler();
        }
        else
        {
            throw "Failed to load texture in " + std::string(Path);
        }
    }
    void CreateSampler()
    {
        VkSamplerCreateInfo sampler = SkInit::samplerCreateInfo();
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.mipLodBias = 0.0f;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        sampler.maxLod = 0.0f;
        if (appBase->deviceFeatures.samplerAnisotropy)
        {
            sampler.maxAnisotropy = appBase->deviceProperties.limits.maxSamplerAnisotropy;
            sampler.anisotropyEnable = VK_TRUE;
        }
        else
        {
            sampler.maxAnisotropy = 1.0;
            sampler.anisotropyEnable = VK_FALSE;
        }
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        VK_CHECK_RESULT(vkCreateSampler(appBase->device, &sampler, nullptr, &this->sampler));
    }

    //You should crete Image before invoking this!
    VkImageView GetImageView()
    {
        if (view != VK_NULL_HANDLE)
        {
            return view;
        }
        assert(this->image != VK_NULL_HANDLE);
        VkImageViewCreateInfo view = SkInit::imageViewCreateInfo();

        view.format = appBase->colorFormat;
        view.components = {VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_A};
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.baseMipLevel = 0;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 1;
        view.subresourceRange.levelCount = 1;
        view.image = this->image;
        VK_CHECK_RESULT(vkCreateImageView(appBase->device, &view, nullptr, &this->view));
        return this->view;
    }
    VkExtent2D GetExtent2D()
    {
        return VkExtent2D{width, height};
    }
    VkExtent3D GetExtent3D()
    {
        return VkExtent3D{width, height, 1};
    }
    void CleanUp()
    {
        fprintf(stderr,"SkTexture::CleanUp...\n");
        
        vkDestroyImageView(appBase->device, this->view, nullptr);
		vkDestroyImage(appBase->device, this->image, nullptr);
		vkDestroySampler(appBase->device, this->sampler, nullptr);
		vkFreeMemory(appBase->device, this->deviceMemory, nullptr);
    }
};

SkTexture::SkTexture(/* args */)
{
}

SkTexture::~SkTexture()
{
}
