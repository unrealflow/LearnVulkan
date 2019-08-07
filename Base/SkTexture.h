#pragma once
#include "SkBase.h"

class SkTexture
{
private:

public:
    unsigned char *data;
    // VkSampler sampler;
    // VkImage image = VK_NULL_HANDLE;
    VkImageLayout layout;
    // VkDeviceMemory deviceMemory;
    // VkDescriptorSet descriptorSet;
    // VkImageView view = VK_NULL_HANDLE;
    SkImage image;
    uint32_t width, height;
    int nrChannels;
    // uint32_t mipLevels;
    // VkFormat format;

    SkTexture(/* args */){}
    ~SkTexture(){}
    void Init(std::string Path)
    {
        int _width, _height;
        data = stbi_load(Path.c_str(), &_width, &_height, &nrChannels, 4);
        width = static_cast<uint32_t>(_width);
        height = static_cast<uint32_t>(_height);
        image.format = VK_FORMAT_R8G8B8A8_UNORM;
        fprintf(stderr, "channels:%d...\n", nrChannels);

        if (!data)
        {
            throw std::runtime_error("Failed to load texture in " + Path);
        }
    }

    VkExtent2D GetExtent2D()
    {
        return VkExtent2D{width, height};
    }
    VkExtent3D GetExtent3D()
    {
        return VkExtent3D{width, height, 1};
    }
};


