#pragma once
#include "SkBase.h"

class SkTexture
{
private:
    SkBase *appBase;
    

public:
    unsigned char *data;
    VkSampler sampler;
    VkImage image = VK_NULL_HANDLE;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
    VkDescriptorSet descriptorSet;
    VkImageView view = VK_NULL_HANDLE;
    uint32_t width, height;
    int nrChannels;
    // uint32_t mipLevels;
    VkFormat format;

    SkTexture(/* args */){}
    ~SkTexture(){}
    void Init(SkBase *initBase, std::string Path)
    {
        appBase = initBase;
        int _width, _height;
        data = stbi_load(Path.c_str(), &_width, &_height, &nrChannels, 4);
        width = static_cast<uint32_t>(_width);
        height = static_cast<uint32_t>(_height);
        format = VK_FORMAT_R8G8B8A8_UNORM;
        fprintf(stderr, "channels:%d...\n", nrChannels);

        if (data)
        {
            CreateSampler();
        }
        else
        {
            throw std::runtime_error("Failed to load texture in " + Path);
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
        sampler.maxAnisotropy = 1.0;
        sampler.anisotropyEnable = VK_FALSE;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        VK_CHECK_RESULT(vkCreateSampler(appBase->device, &sampler, nullptr, &this->sampler));
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
        this->view=VK_NULL_HANDLE;
		vkDestroyImage(appBase->device, this->image, nullptr);
		vkDestroySampler(appBase->device, this->sampler, nullptr);
		vkFreeMemory(appBase->device, this->deviceMemory, nullptr);
    }
};


