#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <optional>
#include <set>
#include "SkTools.h"

//默认窗口大小
const int WIDTH = 960;
const int HEIGHT = 540;

//默认启用的设备扩展
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

//默认启用的校验层
const std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

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
struct SkImage
{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkDescriptorImageInfo descriptor;
    VkFormat format;
};
struct SkBuffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor;
    // VkBufferUsageFlags usage;
    // VkMemoryPropertyFlags mFlags;
    VkDeviceSize size;
    void *data = nullptr;
};
enum LOC
{
    //不同材质预留的binding步长
    STRIDE = 5,
    LIGHT = 50,
    //SkMat属性
    UNIFORM = 10,
    //漫反射贴图
    DIFFUSE = 20,
    //顶点数据
    VERTEX = 12,
    //索引数据
    INDEX = 13,
};

struct BMaterial
{
    glm::vec4 baseColor;
    float metallic;
    float roughness;
    float transmission;
};
struct BTransform
{
    glm::vec3 Position;
    glm::vec3 Rotation;
    glm::vec3 Scale;
};
struct BMesh
{
    float *V;
    uint32_t Vc;
    uint32_t *I;
    uint32_t Ic;
    BTransform *T;
    BMaterial *M;
};
struct BLight
{
    float type;
    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 color;
    float radius;
    float atten;
};
struct BScene
{
    BMesh *meshes;
    uint32_t nums;
    BLight *lights;
    uint32_t lightCount;
};
