#pragma once
#include "SkCommon.h"
#include "Camera.h"
#include "SkInitalizers.h"
#include "stb_image.h"

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
        bool vsync = true;
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
    VkInstance instance = VK_NULL_HANDLE;
    //物理设备(GPU)
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
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
    VkDevice device = VK_NULL_HANDLE;
    //窗口表面
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    //处理队列
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    //呈现队列
    VkQueue presentQueue = VK_NULL_HANDLE;
    QueueFamilyIndices familyIndices;
    //交换链支持的颜色格式
    VkFormat colorFormat;
    VkColorSpaceKHR colorSpace;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    SwapChainSupportDetails swapChainSupport;
    uint32_t imageCount;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    // uint32_t queueNodeIndex = UINT32_MAX;

    //用于储存GBuffer
    SkImage position, normal, albedo, depthStencil;
    //作为后处理阶段的输入
    SkImage post0, post1;
    //深度缓冲格式
    // VkFormat depthFormat;
    //指令池
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    //需要等待队列提交的管线阶段
    VkPipelineStageFlags submintPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //提交的信息，包括帧缓冲和与队列通信的信号量
    VkSubmitInfo submitInfo;
    //用于绘制的指令缓冲
    std::vector<VkCommandBuffer> drawCmdBuffers;
    //用于写入帧缓冲的全局渲染流程
    VkRenderPass renderPass = VK_NULL_HANDLE;
    //可用的帧缓冲
    std::vector<VkFramebuffer> frameBuffers;
    //当前使用的帧缓冲索引
    uint32_t currentFrame = 0;

    // VkPipeline gBufferPipeline = VK_NULL_HANDLE;
    // VkPipeline denoisePipeline = VK_NULL_HANDLE;
    // VkPipelineLayout pipelineLayout;
    // VkDescriptorSet descriptorSet;

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
    VkClearColorValue defaultClearColor = {{0.025f, 0.125f, 0.025f, 1.0f}};
    // Defines a frame rate independent timer value clamped from -1.0...1.0
    // For use in animations, rotations, etc.
    float timer = 0.0f;
    // Multiplier for speeding up (or slowing down) the global timer
    float timerSpeed = 0.25f;

    bool paused = false;
    float currentTime = 0;
    float lastTime = 0;
    float deltaTime = 0;
    // Use to adjust mouse rotation speed
    // float rotationSpeed = 1.0f;
    // Use to adjust mouse zoom speed
    // float zoomSpeed = 1.0f;
    Camera camera;
    //view 和 projection 矩阵对应缓冲
    SkBuffer UBO;
    struct
    {
        glm::mat4 model=glm::mat4(1.0f);
        glm::mat4 view=glm::mat4(1.0f);
        glm::mat4 proj=glm::mat4(1.0f);
        glm::mat4 jitterProj=glm::mat4(1.0f);
        glm::mat4 preView=glm::mat4(1.0f);
        glm::mat4 preProj=glm::mat4(1.0f);
        glm::mat4 viewInverse=glm::mat4(1.0f);
        glm::mat4 projInverse=glm::mat4(1.0f);
        float iTime=0.0f;
        float delta=1.0f;
        float upTime=0.0f;
        uint32_t lightCount=0;
    } uboVS;
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
    SkBase(/* args */) {}
    ~SkBase() {}
};
