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
    //�ж��������ͼ�Ƿ����
    bool viewUpdated = false;
    //��������ʱ������Ŀ���С
    uint32_t destWidth;
    uint32_t destHeight;
    //�����Ƿ���Ҫ����
    bool resizing = false;
    bool prepare = false;
    uint32_t width = WIDTH;
    uint32_t height = HEIGHT;
    struct Settings
    {
        //�Ƿ�����У���
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

    //֡�ʼ���
    uint32_t frameCounter = 0;
    uint32_t lastFPS = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;
    //Skʵ��
    VkInstance instance;
    //�����豸(GPU)
    VkPhysicalDevice physicalDevice;
    //Gpu������
    VkPhysicalDeviceProperties deviceProperties;
    //�豸����
    VkPhysicalDeviceFeatures deviceFeatures;
    //�Դ�����
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    //������Ҫ���豸����
    VkPhysicalDeviceFeatures enableFeatures{};
    //��Ҫ���õ���չ
    std::vector<const char *> enableDeviceExtensions;
    std::vector<const char *> enableInstanceExtensions;
    //�߼��豸������ͬ��Gpu������������Խ����Gpu����ɵ����߼��豸
    VkDevice device;
    //���ڱ���
    VkSurfaceKHR surface;
    //�豸��ʾ���У����ڴ����ύ���豸���壻
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
    //��Ȼ����ʽ
    // VkFormat depthFormat;
    //ָ���
    VkCommandPool cmdPool;
    //��Ҫ�ȴ������ύ�Ĺ��߽׶�
    VkPipelineStageFlags submintPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //�ύ����Ϣ������֡����������ͨ�ŵ��ź���
    VkSubmitInfo submitInfo;
    //���ڻ��Ƶ�ָ���
    std::vector<VkCommandBuffer> drawCmdBuffers;
    //����д��֡�����ȫ����Ⱦ����
    VkRenderPass renderPass;
    //���õ�֡����
    std::vector<VkFramebuffer> frameBuffers;
    //��ǰʹ�õ�֡��������
    uint32_t currentFrame = 0;

    VkPipeline gBufferPipeline = VK_NULL_HANDLE;
    VkPipeline denoisePipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSet;
    //��¼Shaderģ�飬�������ú�����
    std::vector<VkShaderModule> shaderModules;
    //���߻���
    VkPipelineCache pipelineCache;
    //����ͬ�����ź���
    struct
    {
        //������ͼ���ύ
        VkSemaphore presentComplete;
        //ָ����ύ��ִ��
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
