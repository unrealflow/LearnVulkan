#pragma once
#include "SkBase.h"
#include "stb_image.h"
class SkTexture
{
private:
public:
    unsigned char *data;
    VkSampler sampler;
    VkImage image;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
    VkImageView view;
    uint32_t width, height;
    int nrChannels;
    uint32_t mipLevels;
    VkFormat format;

    SkTexture(/* args */);
    ~SkTexture();
    void Init(const char *Path)
    {
        int _width, _height;
        data = stbi_load(Path, &_width, &_height, &nrChannels, 0);
        width = static_cast<uint32_t>(_width);
        height = static_cast<uint32_t>(_height);
        format = SkTools::ConvertFormat(nrChannels);
        assert(format != VK_FORMAT_UNDEFINED);
        if (data)
        {
        }
        else
        {
            throw "Failed to load texture in " + std::string(Path);
        }
    }
    VkExtent2D GetExtent2D()
    {
        return VkExtent2D{width,height};
    }
    VkExtent3D GetExtent3D()
    {
        return VkExtent3D{width,height,1};
    }
};

SkTexture::SkTexture(/* args */)
{
}

SkTexture::~SkTexture()
{
}
