#pragma once
#include "SkCommom.h"
#include "SkInitalizers.h"
#include <array>
#include <chrono>
#include <optional>
#include <set>
#include <glm/glm.hpp>
#include "stb_image.h"
typedef struct _SwapChainBuffer
{
    VkImage image;
    VkImageView view;
} SwapChainBuffer;

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class SkBase
{

public:
    //判断摄像机视图是否更新
    bool viewUpdated = false;
    //调整窗口时，设置目标大小
    int32_t destWidth;
    int32_t destHeight;
    //窗口是否需要调整
    bool resizing = false;
    bool prepare=false;
    uint32_t width = WIDTH;
    uint32_t height = HEIGHT;
    struct Settings
    {
        //是否启用校验层
        bool validation = false;
        /** @brief Set to true if fullscreen mode has been requested via command line */
        bool fullscreen = false;
        /** @brief Set to true if v-sync will be forced for the swapchain */
        bool vsync = false;
        /** @brief Enable UI overlay */
        bool overlay = false;
        std::string name = "";
    } settings;
    GLFWwindow *window;

    //帧率计数
    uint32_t frameCounter = 0;
    uint32_t lastFPS = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;
    //Sk实例
    VkInstance instance;
    //物理设备(GPU)
    VkPhysicalDevice physicalDevice;
    //Gpu的属性
    VkPhysicalDeviceProperties deviceProperties;
    //设备特征
    VkPhysicalDeviceFeatures deviceFeatures;
    //显存属性
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    //程序需要的设备属性
    VkPhysicalDeviceFeatures enableFeatures{};
    //需要启用的扩展
    std::vector<const char *> enableDeviceExtensions;
    std::vector<const char *> enableInstanceExtensions;
    //逻辑设备，将不同的Gpu抽象出来，可以将多个Gpu抽象成单个逻辑设备
    VkDevice device;
    //窗口表面
    VkSurfaceKHR surface;
    //设备显示队列，用于处理提交的设备缓冲；
    // VkQueue queue;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    QueueFamilyIndices familyIndices;
    VkFormat colorFormat;
    VkColorSpaceKHR colorSpace;
    
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    SwapChainSupportDetails swapChainSupport;
    uint32_t imageCount;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    // uint32_t queueNodeIndex = UINT32_MAX;

    //深度缓冲格式
    VkFormat depthFormat;
    //指令池
    VkCommandPool cmdPool;
    //需要等待队列提交的管线阶段
    VkPipelineStageFlags submintPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //提交的信息，包括帧缓冲和与队列通信的信号量
    VkSubmitInfo submitInfo;
    //用于绘制的指令缓冲
    std::vector<VkCommandBuffer> drawCmdBuffers;
    //用于写入帧缓冲的全局渲染流程
    VkRenderPass renderPass;
    //可用的帧缓冲
    std::vector<VkFramebuffer> frameBuffers;
    //当前使用的帧缓冲索引
    // uint32_t currentBufferIndex = 0;
    
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSet;
    //记录Shader模块，便于重用和清理
    std::vector<VkShaderModule> shaderModules;
    //管线缓存
    VkPipelineCache pipelineCache;
    //用于同步的信号量
    struct
    {
        //交换链图像提交
        VkSemaphore presentComplete;
        //指令缓冲提交并执行
        VkSemaphore renderComplete;
    } semaphores;
    std::vector<VkFence> waitFences;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkExtent2D getExtent()
    {
        return VkExtent2D{width,height};
    }
    void setExtent(VkExtent2D extent)
    {
        width=extent.width;
        height=extent.height;
    };
    SkBase(/* args */);
    ~SkBase();
};

SkBase::SkBase(/* args */)
{
}

SkBase::~SkBase()
{
}
