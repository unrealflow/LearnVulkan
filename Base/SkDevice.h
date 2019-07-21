#pragma once
#include "SkBase.h"
#include "SkTools.h"
#include "SkDebug.h"

class SkDevice
{
private:
    SkBase *appBase;
    /* data */
    void pickPhysicalDevice(bool CheckDevice = true)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(appBase->instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(appBase->instance, &deviceCount, devices.data());
        if (CheckDevice)
        {
            for (const auto &device : devices)
            {
                if (isDeviceSuitable(device))
                {
                    appBase->physicalDevice = device;
                    break;
                }
            }
        }
        else
        {
            appBase->physicalDevice = devices[0];
        }
        if (appBase->physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        appBase->swapChainSupport = querySwapChainSupport(appBase->physicalDevice);
        vkGetPhysicalDeviceFeatures(appBase->physicalDevice, &(appBase->deviceFeatures));
        vkGetPhysicalDeviceProperties(appBase->physicalDevice, &(appBase->deviceProperties));
        vkGetPhysicalDeviceMemoryProperties(appBase->physicalDevice, &(appBase->deviceMemoryProperties));
        fprintf(stderr, "Select Device:\t%s...\n", appBase->deviceProperties.deviceName);
    }
    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

            if (indices.isComplete() && swapChainAdequate)
            {
                return true;
            }
        }
        return false;
    }

    void createLogicalDevice()
    {
        // QueueFamilyIndices indices = findQueueFamilies(appBase->physicalDevice);
        appBase->familyIndices = findQueueFamilies(appBase->physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {appBase->familyIndices.graphicsFamily.value(), appBase->familyIndices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (appBase->settings.validation)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(appBase->physicalDevice, &createInfo, nullptr, &appBase->device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(appBase->device, appBase->familyIndices.graphicsFamily.value(), 0, &(appBase->graphicsQueue));
        vkGetDeviceQueue(appBase->device, appBase->familyIndices.presentFamily.value(), 0, &(appBase->presentQueue));
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto &extension : availableExtensions)
        {
            //--------------------------
            // printf("\t\t%s\n",extension.extensionName);
            //--------------------------
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
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

public:
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, appBase->surface, &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, appBase->surface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, appBase->surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, appBase->surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, appBase->surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, appBase->surface, &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport)
            {
                indices.presentFamily = i;
            }
            if (indices.isComplete())
            {
                break;
            }
            i++;
        }

        return indices;
    }
    SkDevice()
    {
    }
    void Init(SkBase *initBase)
    {
        fprintf(stderr, "SkDevice::Init...\n");
        appBase = initBase;
        VK_CHECK_RESULT(glfwCreateWindowSurface(appBase->instance, appBase->window, nullptr, &(appBase->surface)));
        pickPhysicalDevice(false);
        createLogicalDevice();
    }
    void CleanUp()
    {
        fprintf(stderr, "SkDevice::CleanUp...\n");
        vkDestroyDevice(appBase->device, nullptr);
    }
    ~SkDevice()
    {
    }
    void CreateBuffer(const void *initData, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer *outBuffer, VkDeviceMemory *outMemory)
    {

        void *data;
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
        // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is host visible memory, and VK_MEMORY_PROPERTY_HOST_COHERENT_BIT makes sure writes are directly visible
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(appBase->device, &memAlloc, nullptr, outMemory));
        VK_CHECK_RESULT(vkMapMemory(appBase->device, *outMemory, 0, memAlloc.allocationSize, 0, &data));
        memcpy(data, initData, size);
        vkUnmapMemory(appBase->device, *outMemory);
        VK_CHECK_RESULT(vkBindBufferMemory(appBase->device, *outBuffer, *outMemory, 0));
    }
    void CreateLocalBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer *outBuffer, VkDeviceMemory *outMemory)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VkMemoryAllocateInfo memAlloc = {};
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

        VkMemoryRequirements memReqs;
        VK_CHECK_RESULT(vkCreateBuffer(appBase->device, &bufferInfo, nullptr, outBuffer));
        vkGetBufferMemoryRequirements(appBase->device, *outBuffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(appBase->device, &memAlloc, nullptr, outMemory));
        VK_CHECK_RESULT(vkBindBufferMemory(appBase->device, *outBuffer, *outMemory, 0));
    }
};
