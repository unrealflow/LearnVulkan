#pragma once
#include "SkBase.h"
#include "SkTools.h"
#include "SkDebug.h"
#include "SkDevice.h"




class SkSwapChain
{
private:
    SkBase *appBase;
    GLFWwindow *window;
    SkDevice *pDevice;
    void createSwapChain(VkSwapchainKHR _oldSwapChain=VK_NULL_HANDLE)
    {
        SwapChainSupportDetails swapChainSupport =pDevice->querySwapChainSupport(appBase->physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        appBase->imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && appBase->imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            appBase->imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = appBase->surface;

        createInfo.minImageCount = appBase->imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = pDevice->findQueueFamilies(appBase->physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = _oldSwapChain;

        VK_CHECK_RESULT(vkCreateSwapchainKHR(appBase->device, &createInfo, nullptr, &(appBase->swapChain)));

        if(_oldSwapChain!=VK_NULL_HANDLE)
        {
            CleanSwapChain(_oldSwapChain);
        }
        VK_CHECK_RESULT(vkGetSwapchainImagesKHR(appBase->device, appBase->swapChain, &(appBase->imageCount), nullptr));
        appBase->images.resize(appBase->imageCount);
        VK_CHECK_RESULT(vkGetSwapchainImagesKHR(appBase->device, appBase->swapChain, &(appBase->imageCount), appBase->images.data()));

        appBase->colorFormat = surfaceFormat.format;
        appBase->extent = extent;
        
    }
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
        if (!appBase->settings.vsync)
        {
            for (const auto &availablePresentMode : availablePresentModes)
            {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    return availablePresentMode;
                }
                else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
                {
                    bestMode = availablePresentMode;
                }
            }
        }
        return bestMode;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = {WIDTH, HEIGHT};

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }
    void createImageViews()
    {
        fprintf(stderr,"imageCount:%d...\n",appBase->imageCount);
        
        appBase->imageViews.resize(appBase->imageCount);
        for (uint32_t i = 0; i < appBase->imageCount; i++)
		{
			VkImageViewCreateInfo colorAttachmentView = {};
			colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorAttachmentView.pNext = NULL;
			colorAttachmentView.format = appBase->colorFormat;
			colorAttachmentView.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;
			colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorAttachmentView.flags = 0;

			colorAttachmentView.image = appBase->images[i];

			VK_CHECK_RESULT(vkCreateImageView(appBase->device, &colorAttachmentView, nullptr, &(appBase->imageViews[i])));
		}
    }
    void CleanSwapChain(VkSwapchainKHR toClean)
    {
        for (uint32_t i = 0; i < appBase->imageCount; i++)
			{
				vkDestroyImageView(appBase->device, appBase->imageViews[i], nullptr);
			}
			vkDestroySwapchainKHR(appBase->device, toClean, nullptr);
    }

public:
    void Init(SkBase *initBase,SkDevice *initDevice)
    {
        fprintf(stderr,"SkSwapChain::Init...\n");
        appBase=initBase;
        pDevice=initDevice;
        createSwapChain();
        createImageViews();
    }
    void Create(uint32_t width, uint32_t height, bool _vsync = false)
    {
        fprintf(stderr,"SkSwapChain::Create...\n");

        appBase->extent.width=width;
        appBase->extent.height=height;
        appBase->settings.vsync=_vsync;
        createSwapChain(appBase->swapChain);
        
    }
    void CleanUp()
    {
        fprintf(stderr,"SkSwapChain::CleanUp...\n");
        CleanSwapChain(appBase->swapChain);
        vkDestroySwapchainKHR(appBase->device, appBase->swapChain, nullptr);
        vkDestroySurfaceKHR(appBase->instance, appBase->surface, nullptr);
    }
};