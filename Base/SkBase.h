#pragma once
#include "SkCommom.h"
#include <array>
#include <chrono>
#include <optional>
#include <set>
#include <glm/glm.hpp>
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
    //�ж��������ͼ�Ƿ����
    bool viewUpdated = false;
    //��������ʱ������Ŀ���С
    int32_t destWidth;
    int32_t destHeight;
    //�����Ƿ���Ҫ����
    bool resizing = false;
    uint32_t WIDTH = 800;
    uint32_t HEIGHT = 600;
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
    VkQueue queue;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkFormat colorFormat;
    VkColorSpaceKHR colorSpace;
    VkExtent2D extent;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    uint32_t imageCount;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    // std::vector<SwapChainBuffer> buffers;
    uint32_t queueNodeIndex = UINT32_MAX;

    //��Ȼ����ʽ
    VkFormat depthFormat;
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
    uint32_t currentBuffer = 0;
    //��������
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
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

    SkBase(/* args */);
    ~SkBase();
};

SkBase::SkBase(/* args */)
{
}

SkBase::~SkBase()
{
}
