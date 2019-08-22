#pragma once
#include "SkBase.h"

class SkInstance
{
private:
    /* data */
    SkBase *appBase;
    void CreateInstance()
    {
        if (appBase->settings.validation && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appBase->settings.name.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.pEngineName = appBase->settings.name.c_str();
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &appInfo;

        auto extensions = GetRequiredExtensions();

        for (auto &&i : appBase->enableInstanceExtensions)
        {
            extensions.emplace_back(i);
        }

        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceInfo.ppEnabledExtensionNames = extensions.data();

        if (appBase->settings.validation)
        {
            instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            instanceInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            instanceInfo.enabledLayerCount = 0;
        }
        VK_CHECK_RESULT(vkCreateInstance(&instanceInfo, nullptr, &appBase->instance));
    }

    void SetupDebugMessenger()
    {
        if (appBase->settings.validation)
        {
            fprintf(stderr, "Setup Debug Message...\n");
            VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;

            if (CreateDebugUtilsMessengerEXT(appBase->instance, &createInfo, nullptr, &(appBase->debugMessenger)) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }
    }
     bool CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : validationLayers)
        {
            bool layerFound = false;
            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound)
            {
                return false;
            }
        }
        return true;
    }

    std::vector<const char *> GetRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (appBase->settings.validation)
        {
            extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }
    void InitWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        appBase->window = glfwCreateWindow(appBase->width, appBase->height, appBase->settings.name.c_str(), nullptr, nullptr);
    }
public:
    void Init(SkBase *initBase)
    {
        fprintf(stderr,"SkInstance::Init...\n");
        
        appBase=initBase;
        InitWindow();
        CreateInstance();
        SetupDebugMessenger();
    }
    void CleanUp()
    {
        
        if (appBase->settings.validation)
        {
            DestroyDebugUtilsMessengerEXT(appBase->instance, appBase->debugMessenger, nullptr);
        }
        vkDestroyInstance(appBase->instance, nullptr);
        glfwDestroyWindow(appBase->window);
        glfwTerminate();
    }
    
};

