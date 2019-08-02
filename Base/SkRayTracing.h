#pragma once
#include "SkBase.h"
#include "SkMemory.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Ray tracing acceleration structure
struct AccelerationStructure
{
    VkDeviceMemory memory;
    VkAccelerationStructureNV accelerationStructure;
    uint64_t handle;
};

// Ray tracing geometry instance
struct GeometryInstance
{
    glm::mat3x4 transform;
    uint32_t instanceId : 24;
    uint32_t mask : 8;
    uint32_t instanceOffset : 24;
    uint32_t flags : 8;
    uint64_t accelerationStructureHandle;
};

// Indices for the different ray tracing shader types used in this example
#define INDEX_RAYGEN 0
#define INDEX_MISS 1
#define INDEX_CLOSEST_HIT 2

class SkRayTracing
{
private:
    SkBase *appBase;
    SkMemory *mem;

public:
    SkRayTracing(/* args */) {}
    ~SkRayTracing() {}
    VkPhysicalDeviceRayTracingPropertiesNV rayTracingProperties{};

    AccelerationStructure bottomLevelAS;
    AccelerationStructure topLevelAS;
    SkImage storageImage;

    struct UniformData
    {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
    } uniformDataRT;
    struct
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkDescriptorBufferInfo descriptor;
    } uniformBufferRT;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
    void Init(SkBase *initBase, SkMemory *initMem)
    {
        appBase = initBase;
        mem = initMem;
        rayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
		VkPhysicalDeviceProperties2 deviceProps2{};
		deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		deviceProps2.pNext = &rayTracingProperties;
		vkGetPhysicalDeviceProperties2(appBase->physicalDevice, &deviceProps2);
    }
    void CreateStorageImage()
    {
        mem->CreateStorageImage(&storageImage);
    }
};
