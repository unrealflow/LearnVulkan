#pragma once
#include "SkCommom.h"
#include "Camera.h"
#include "SkInitalizers.h"
#include <array>
#include <chrono>
#include <optional>
#include <set>
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
struct FrameBufferAttachment
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkFormat format;
};
class SkBase
{

public:
    //判断摄像机视图是否更新
    bool viewUpdated = false;
    //调整窗口时，设置目标大小
    uint32_t destWidth;
    uint32_t destHeight;
    //窗口是否需要调整
    bool resizing = false;
    bool prepare = false;
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
    FrameBufferAttachment position, normal, albedo, depthStencil;
    //深度缓冲格式
    // VkFormat depthFormat;
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
    uint32_t currentFrame = 0;

    VkPipeline gBufferPipeline = VK_NULL_HANDLE;
    VkPipeline denoisePipeline = VK_NULL_HANDLE;
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
    VkClearColorValue defaultClearColor = {{0.025f, 0.025f, 0.025f, 1.0f}};
    // Defines a frame rate independent timer value clamped from -1.0...1.0
    // For use in animations, rotations, etc.
    float timer = 0.0f;
    // Multiplier for speeding up (or slowing down) the global timer
    float timerSpeed = 0.25f;

    bool paused = false;
    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;
    // Use to adjust mouse rotation speed
    // float rotationSpeed = 1.0f;
    // Use to adjust mouse zoom speed
    // float zoomSpeed = 1.0f;
    Camera camera;

    // glm::vec3 rotation = glm::vec3();
    // glm::vec3 cameraPos = glm::vec3();
    // glm::vec2 mousePos;

    VkExtent2D getExtent()
    {
        return VkExtent2D{width, height};
    }
    VkExtent3D getExtent3D()
    {
        return VkExtent3D{width, height, 1};
    }
    void setExtent(VkExtent2D extent)
    {
        width = extent.width;
        height = extent.height;
    };
    float GetAspect()
    {
        return static_cast<float>(width) / height;
    }
    SkBase(/* args */);
    ~SkBase();
};

SkBase::SkBase(/* args */)
{
}

SkBase::~SkBase()
{
}
