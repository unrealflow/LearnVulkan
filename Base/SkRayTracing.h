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
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkAccelerationStructureNV accelerationStructure = VK_NULL_HANDLE;
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
#define INDEX_SHADOW_MISS 2
#define INDEX_CLOSEST_HIT 3
#define INDEX_SHADOW_HIT 4

#define NUM_SHADER_GROUPS 5

const std::vector<const char *> RTInstanceExtensions = {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
const std::vector<const char *> RTDeviceExtensions = {VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, VK_NV_RAY_TRACING_EXTENSION_NAME};

class SkRayTracing
{
private:
    SkBase *appBase;
    SkMemory *mem;
    void Prepare()
    {
        rayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
        VkPhysicalDeviceProperties2 deviceProps2{};
        deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProps2.pNext = &rayTracingProperties;
        vkGetPhysicalDeviceProperties2(appBase->physicalDevice, &deviceProps2);

        // Get VK_NV_ray_tracing related function pointers
        vkCreateAccelerationStructureNV = reinterpret_cast<PFN_vkCreateAccelerationStructureNV>(vkGetDeviceProcAddr(appBase->device, "vkCreateAccelerationStructureNV"));
        vkDestroyAccelerationStructureNV = reinterpret_cast<PFN_vkDestroyAccelerationStructureNV>(vkGetDeviceProcAddr(appBase->device, "vkDestroyAccelerationStructureNV"));
        vkBindAccelerationStructureMemoryNV = reinterpret_cast<PFN_vkBindAccelerationStructureMemoryNV>(vkGetDeviceProcAddr(appBase->device, "vkBindAccelerationStructureMemoryNV"));
        vkGetAccelerationStructureHandleNV = reinterpret_cast<PFN_vkGetAccelerationStructureHandleNV>(vkGetDeviceProcAddr(appBase->device, "vkGetAccelerationStructureHandleNV"));
        vkGetAccelerationStructureMemoryRequirementsNV = reinterpret_cast<PFN_vkGetAccelerationStructureMemoryRequirementsNV>(vkGetDeviceProcAddr(appBase->device, "vkGetAccelerationStructureMemoryRequirementsNV"));
        vkCmdBuildAccelerationStructureNV = reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(vkGetDeviceProcAddr(appBase->device, "vkCmdBuildAccelerationStructureNV"));
        vkCreateRayTracingPipelinesNV = reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(vkGetDeviceProcAddr(appBase->device, "vkCreateRayTracingPipelinesNV"));
        vkGetRayTracingShaderGroupHandlesNV = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesNV>(vkGetDeviceProcAddr(appBase->device, "vkGetRayTracingShaderGroupHandlesNV"));
        vkCmdTraceRaysNV = reinterpret_cast<PFN_vkCmdTraceRaysNV>(vkGetDeviceProcAddr(appBase->device, "vkCmdTraceRaysNV"));
    }
    void CreateBottomLevelAccelerationStructure(const VkGeometryNV *geometries)
    {
        VkAccelerationStructureInfoNV accelerationStructureInfo{};
        accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
        accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        accelerationStructureInfo.instanceCount = 0;
        accelerationStructureInfo.geometryCount = 1;
        accelerationStructureInfo.pGeometries = geometries;

        VkAccelerationStructureCreateInfoNV accelerationStructureCI{};
        accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
        accelerationStructureCI.info = accelerationStructureInfo;
        VK_CHECK_RESULT(vkCreateAccelerationStructureNV(appBase->device, &accelerationStructureCI, nullptr, &bottomLevelAS.accelerationStructure));

        VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{};
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        memoryRequirementsInfo.accelerationStructure = bottomLevelAS.accelerationStructure;

        VkMemoryRequirements2 memoryRequirements2{};
        vkGetAccelerationStructureMemoryRequirementsNV(appBase->device, &memoryRequirementsInfo, &memoryRequirements2);

        VkMemoryAllocateInfo memoryAllocateInfo = SkInit::memoryAllocateInfo();
        memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = mem->GetMemoryTypeIndex(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(appBase->device, &memoryAllocateInfo, nullptr, &bottomLevelAS.memory));

        VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo{};
        accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
        accelerationStructureMemoryInfo.accelerationStructure = bottomLevelAS.accelerationStructure;
        accelerationStructureMemoryInfo.memory = bottomLevelAS.memory;
        VK_CHECK_RESULT(vkBindAccelerationStructureMemoryNV(appBase->device, 1, &accelerationStructureMemoryInfo));

        VK_CHECK_RESULT(vkGetAccelerationStructureHandleNV(appBase->device, bottomLevelAS.accelerationStructure, sizeof(uint64_t), &bottomLevelAS.handle));
    }
    void CreateTopLevelAccelerationStructure()
    {
        VkAccelerationStructureInfoNV accelerationStructureInfo{};
        accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
        accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
        accelerationStructureInfo.instanceCount = 1;
        accelerationStructureInfo.geometryCount = 0;

        VkAccelerationStructureCreateInfoNV accelerationStructureCI{};
        accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
        accelerationStructureCI.info = accelerationStructureInfo;
        VK_CHECK_RESULT(vkCreateAccelerationStructureNV(appBase->device, &accelerationStructureCI, nullptr, &topLevelAS.accelerationStructure));

        VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{};
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        memoryRequirementsInfo.accelerationStructure = topLevelAS.accelerationStructure;

        VkMemoryRequirements2 memoryRequirements2{};
        vkGetAccelerationStructureMemoryRequirementsNV(appBase->device, &memoryRequirementsInfo, &memoryRequirements2);

        VkMemoryAllocateInfo memoryAllocateInfo = SkInit::memoryAllocateInfo();
        memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = mem->GetMemoryTypeIndex(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(appBase->device, &memoryAllocateInfo, nullptr, &topLevelAS.memory));

        VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo{};
        accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
        accelerationStructureMemoryInfo.accelerationStructure = topLevelAS.accelerationStructure;
        accelerationStructureMemoryInfo.memory = topLevelAS.memory;
        VK_CHECK_RESULT(vkBindAccelerationStructureMemoryNV(appBase->device, 1, &accelerationStructureMemoryInfo));

        VK_CHECK_RESULT(vkGetAccelerationStructureHandleNV(appBase->device, topLevelAS.accelerationStructure, sizeof(uint64_t), &topLevelAS.handle));
    }
    std::vector<VkShaderModule> shaderModules;
    inline VkPipelineShaderStageCreateInfo CreateShaderStageInfo(std::string path, VkShaderStageFlagBits stage)
    {
        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = stage;
        shaderStageInfo.module = SkTools::CreateShaderModule(appBase->device, path);
        shaderStageInfo.pName = "main";
        shaderModules.push_back(shaderStageInfo.module);
        return shaderStageInfo;
    }

public:
    SkRayTracing(/* args */) {}
    ~SkRayTracing() {}
    PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV;
    PFN_vkDestroyAccelerationStructureNV vkDestroyAccelerationStructureNV;
    PFN_vkBindAccelerationStructureMemoryNV vkBindAccelerationStructureMemoryNV;
    PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV;
    PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV;
    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV;
    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV;
    PFN_vkGetRayTracingShaderGroupHandlesNV vkGetRayTracingShaderGroupHandlesNV;
    PFN_vkCmdTraceRaysNV vkCmdTraceRaysNV;

    VkPhysicalDeviceRayTracingPropertiesNV rayTracingProperties{};

    AccelerationStructure bottomLevelAS;
    AccelerationStructure topLevelAS;
    SkImage storageImage;
    VkSampler sampler;
    struct
    {
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
    } instanceBuffer, scratchBuffer, shaderBindingTable;
    struct UniformData
    {
        glm::mat4 viewInverse = glm::mat4();
        glm::mat4 projInverse = glm::mat4();
        glm::vec4 lightPos;
    } uniformDataRT;
    SkBuffer uniformBufferRT;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    std::vector<VkCommandBuffer> rayCmdBuffers;

    void Init(SkBase *initBase, SkMemory *initMem)
    {
        appBase = initBase;
        mem = initMem;
        shaderModules.clear();
        uniformDataRT.projInverse = glm::inverse(appBase->camera.matrices.perspective);
        uniformDataRT.viewInverse = glm::inverse(appBase->camera.matrices.view);
        uniformDataRT.lightPos = glm::vec4(appBase->camera.position, 1.0f);
        Prepare();
    }
    void CleanUp()
    {
        mem->FreeImage(&storageImage);
        mem->FreeBuffer(&uniformBufferRT);
        mem->FreeBuffer(&shaderBindingTable.buffer, &shaderBindingTable.memory);
        vkDestroySampler(appBase->device, sampler, nullptr);
        mem->FreeShaderModules(shaderModules);
        mem->FreeDescriptorPool(&descriptorPool);
        mem->FreeLayout(&pipelineLayout);
        mem->FreeLayout(&descriptorSetLayout);
        mem->FreePipeline(&pipeline);
        FreeAccelerationStructure(&bottomLevelAS);
        FreeAccelerationStructure(&topLevelAS);
    }
    void CreateStorageImage()
    {
        mem->CreateStorageImage(&storageImage, true);
        mem->CreateSampler(&sampler);
    }
    void FreeAccelerationStructure(AccelerationStructure *as)
    {
        vkFreeMemory(appBase->device, as->memory, nullptr);
        vkDestroyAccelerationStructureNV(appBase->device, as->accelerationStructure, nullptr);
        as->memory = VK_NULL_HANDLE;
        as->accelerationStructure = VK_NULL_HANDLE;
    }
    VkDescriptorImageInfo GetReadDescriptor()
    {
        VkDescriptorImageInfo imageDes = {};
        imageDes.sampler = sampler;
        imageDes.imageView = storageImage.view;
        imageDes.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        return imageDes;
    }

    void CreateScene(SkModel *model)
    {
        VkGeometryNV geometry{};
        geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
        geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
        geometry.geometry.triangles.vertexData = model->vertices.buffer;
        geometry.geometry.triangles.vertexOffset = 0;
        geometry.geometry.triangles.vertexCount = model->GetVertexCount();
        geometry.geometry.triangles.vertexStride = model->vertices.stride;
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        geometry.geometry.triangles.indexData = model->indices.buffer;
        geometry.geometry.triangles.indexOffset = 0;
        geometry.geometry.triangles.indexCount = model->GetIndexCount();
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
        geometry.geometry.triangles.transformOffset = 0;
        geometry.geometry.aabbs = {};
        geometry.geometry.aabbs.sType = {VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV};
        geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;

        CreateBottomLevelAccelerationStructure(&geometry);

        glm::mat4 transform = glm::mat4(1.0f);
        transform=glm::translate(transform,glm::vec3(0.0f,0.0f,1000.0f));
        transform=glm::scale(transform,glm::vec3(0.1f));

        std::array<GeometryInstance, 2> geometryInstances{};
		// First geometry instance is used for the scene hit and miss shaders
		geometryInstances[0].transform = transform;
		geometryInstances[0].instanceId = 0;
		geometryInstances[0].mask = 0xff;
		geometryInstances[0].instanceOffset = 0;
		geometryInstances[0].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
		geometryInstances[0].accelerationStructureHandle = bottomLevelAS.handle;
		// Second geometry instance is used for the shadow hit and miss shaders
		geometryInstances[1].transform = transform;
		geometryInstances[1].instanceId = 1;
		geometryInstances[1].mask = 0xff;
		geometryInstances[1].instanceOffset = 2;
		geometryInstances[1].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
		geometryInstances[1].accelerationStructureHandle = bottomLevelAS.handle;

        mem->CreateBuffer(geometryInstances.data(),
                          sizeof(GeometryInstance)*geometryInstances.size(),
                          VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                          &instanceBuffer.buffer,
                          &instanceBuffer.memory);

        CreateTopLevelAccelerationStructure();

        VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{};
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;

        VkMemoryRequirements2 memReqBottomLevelAS;
        memoryRequirementsInfo.accelerationStructure = bottomLevelAS.accelerationStructure;
        vkGetAccelerationStructureMemoryRequirementsNV(appBase->device, &memoryRequirementsInfo, &memReqBottomLevelAS);

        VkMemoryRequirements2 memReqTopLevelAS;
        memoryRequirementsInfo.accelerationStructure = topLevelAS.accelerationStructure;
        vkGetAccelerationStructureMemoryRequirementsNV(appBase->device, &memoryRequirementsInfo, &memReqTopLevelAS);

        const VkDeviceSize scratchBufferSize = std::max(memReqBottomLevelAS.memoryRequirements.size, memReqTopLevelAS.memoryRequirements.size);

        mem->dCreateBuffer(scratchBufferSize,
                           VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           &scratchBuffer.buffer,
                           &scratchBuffer.memory);
        VkCommandBuffer cmdBuf = mem->GetCommandBuffer(true);

        VkAccelerationStructureInfoNV buildInfo{};
        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
        buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        buildInfo.geometryCount = 1;
        buildInfo.pGeometries = &geometry;

        vkCmdBuildAccelerationStructureNV(
            cmdBuf,
            &buildInfo,
            VK_NULL_HANDLE,
            0,
            VK_FALSE,
            bottomLevelAS.accelerationStructure,
            VK_NULL_HANDLE,
            scratchBuffer.buffer,
            0);
        VkMemoryBarrier memoryBarrier = SkInit::memoryBarrier();
        memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
        memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

        buildInfo.pGeometries = 0;
        buildInfo.geometryCount = 0;
        buildInfo.instanceCount = 1;

        vkCmdBuildAccelerationStructureNV(
            cmdBuf,
            &buildInfo,
            instanceBuffer.buffer,
            0,
            VK_FALSE,
            topLevelAS.accelerationStructure,
            VK_NULL_HANDLE,
            scratchBuffer.buffer,
            0);

        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

        mem->FlushCommandBuffer(cmdBuf);
        mem->FreeBuffer(&instanceBuffer.buffer, &instanceBuffer.memory);
        mem->FreeBuffer(&scratchBuffer.buffer, &scratchBuffer.memory);
    }
    void CreateUniformBuffer()
    {
        mem->CreateBuffer(&uniformDataRT,
                          sizeof(uniformDataRT),
                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          &uniformBufferRT);
        mem->SetupDescriptor(&uniformBufferRT);
        mem->Map(&uniformBufferRT);
    }
    void UpdateUniformBuffers()
    {
        uniformDataRT.projInverse = glm::inverse(appBase->camera.matrices.perspective);
        uniformDataRT.viewInverse = glm::inverse(appBase->camera.matrices.view);
        uniformDataRT.lightPos = glm::vec4(cos(glm::radians(appBase->currentTime)) * 40.0f, -50.0f + sin(glm::radians(appBase->currentTime)) * 20.0f, 25.0f + sin(glm::radians(appBase->currentTime)) * 5.0f, 0.0f);

        assert(uniformBufferRT.data);
        memcpy(uniformBufferRT.data, &uniformDataRT, sizeof(uniformDataRT));
    }
    void CreateRayTracingPipeline()
    {
        VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
        accelerationStructureLayoutBinding.binding = 0;
        accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
        accelerationStructureLayoutBinding.descriptorCount = 1;
        accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

        VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
        resultImageLayoutBinding.binding = 1;
        resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        resultImageLayoutBinding.descriptorCount = 1;
        resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;

        VkDescriptorSetLayoutBinding uniformBufferBinding{};
        uniformBufferBinding.binding = 2;
        uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformBufferBinding.descriptorCount = 1;
        uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

        VkDescriptorSetLayoutBinding vertexBufferBinding{};
        vertexBufferBinding.binding = 3;
        vertexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        vertexBufferBinding.descriptorCount = 1;
        vertexBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

        VkDescriptorSetLayoutBinding indexBufferBinding{};
        indexBufferBinding.binding = 4;
        indexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        indexBufferBinding.descriptorCount = 1;
        indexBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

        std::vector<VkDescriptorSetLayoutBinding> bindings({accelerationStructureLayoutBinding,
                                                            resultImageLayoutBinding,
                                                            uniformBufferBinding,
                                                            vertexBufferBinding,
                                                            indexBufferBinding});

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(appBase->device, &layoutInfo, nullptr, &descriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

        VK_CHECK_RESULT(vkCreatePipelineLayout(appBase->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

        const uint32_t shaderIndexRaygen = 0;
        const uint32_t shaderIndexMiss = 1;
        const uint32_t shaderIndexShadowMiss = 2;
        const uint32_t shaderIndexClosestHit = 3;

        std::array<VkPipelineShaderStageCreateInfo, 4> shaderStages;
        shaderStages[shaderIndexRaygen] = CreateShaderStageInfo("Shader/s3_raygen.spv", VK_SHADER_STAGE_RAYGEN_BIT_NV);
        shaderStages[shaderIndexMiss] = CreateShaderStageInfo("Shader/s3_miss.spv", VK_SHADER_STAGE_MISS_BIT_NV);
        shaderStages[shaderIndexShadowMiss] = CreateShaderStageInfo("Shader/s3_shadow.spv", VK_SHADER_STAGE_MISS_BIT_NV);
        shaderStages[shaderIndexClosestHit] = CreateShaderStageInfo("Shader/s3_closesthit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);

        std::array<VkRayTracingShaderGroupCreateInfoNV, NUM_SHADER_GROUPS> groups{};
        for (auto &group : groups)
        {
            // Init all groups with some default values
            group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            group.generalShader = VK_SHADER_UNUSED_NV;
            group.closestHitShader = VK_SHADER_UNUSED_NV;
            group.anyHitShader = VK_SHADER_UNUSED_NV;
            group.intersectionShader = VK_SHADER_UNUSED_NV;
        }

        // Links shaders and types to ray tracing shader groups
        // Ray generation shader group
        groups[INDEX_RAYGEN].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        groups[INDEX_RAYGEN].generalShader = shaderIndexRaygen;
        // Scene miss shader group
        groups[INDEX_MISS].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        groups[INDEX_MISS].generalShader = shaderIndexMiss;
        // Shadow miss shader group
        groups[INDEX_SHADOW_MISS].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        groups[INDEX_SHADOW_MISS].generalShader = shaderIndexShadowMiss;
        // Scene closest hit shader group
        groups[INDEX_CLOSEST_HIT].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        groups[INDEX_CLOSEST_HIT].generalShader = VK_SHADER_UNUSED_NV;
        groups[INDEX_CLOSEST_HIT].closestHitShader = shaderIndexClosestHit;
        // Shadow closest hit shader group
        groups[INDEX_SHADOW_HIT].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        groups[INDEX_SHADOW_HIT].generalShader = VK_SHADER_UNUSED_NV;
        // Reuse shadow miss shader
        groups[INDEX_SHADOW_HIT].closestHitShader = shaderIndexShadowMiss;

        VkRayTracingPipelineCreateInfoNV rayPipelineInfo{};
        rayPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
        rayPipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        rayPipelineInfo.pStages = shaderStages.data();
        rayPipelineInfo.groupCount = static_cast<uint32_t>(groups.size());
        rayPipelineInfo.pGroups = groups.data();
        rayPipelineInfo.maxRecursionDepth = 2;
        rayPipelineInfo.layout = pipelineLayout;
        VK_CHECK_RESULT(vkCreateRayTracingPipelinesNV(appBase->device, VK_NULL_HANDLE, 1, &rayPipelineInfo, nullptr, &pipeline));
    }
    void CreateShaderBindingTable()
    {
        const uint32_t sbtSize = rayTracingProperties.shaderGroupHandleSize * NUM_SHADER_GROUPS;
        mem->dCreateBuffer(sbtSize,
                           VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                           &shaderBindingTable.buffer,
                           &shaderBindingTable.memory);
        auto shaderHandleStorage = new uint8_t[sbtSize];
        VK_CHECK_RESULT(vkGetRayTracingShaderGroupHandlesNV(appBase->device, pipeline, 0, NUM_SHADER_GROUPS, sbtSize, shaderHandleStorage));
        uint8_t *data;
        void *temp;
        vkMapMemory(appBase->device, shaderBindingTable.memory, 0, sbtSize, 0, &temp);
        data = static_cast<uint8_t *>(temp);
        // Copy the shader identifiers to the shader binding table
        VkDeviceSize offset = 0;
        data += CopyShaderIdentifier(data, shaderHandleStorage, INDEX_RAYGEN);
        data += CopyShaderIdentifier(data, shaderHandleStorage, INDEX_MISS);
        data += CopyShaderIdentifier(data, shaderHandleStorage, INDEX_SHADOW_MISS);
        data += CopyShaderIdentifier(data, shaderHandleStorage, INDEX_CLOSEST_HIT);
        data += CopyShaderIdentifier(data, shaderHandleStorage, INDEX_SHADOW_HIT);
        vkUnmapMemory(appBase->device, shaderBindingTable.memory);
        delete[] shaderHandleStorage;
        shaderHandleStorage = nullptr;
    }
    uint32_t CopyShaderIdentifier(uint8_t *dst, const uint8_t *shaderHandleStorage, uint32_t groupIndex)
    {
        const uint32_t shaderGroupHandleSize = rayTracingProperties.shaderGroupHandleSize;
        memcpy(dst, shaderHandleStorage + groupIndex * shaderGroupHandleSize, shaderGroupHandleSize);
        return shaderGroupHandleSize;
    }
    void CreateDescriptorSets(SkModel *model)
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {
            {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}};
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = SkInit::descriptorPoolCreateInfo(poolSizes, 1);
        VK_CHECK_RESULT(vkCreateDescriptorPool(appBase->device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = SkInit::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
        VK_CHECK_RESULT(vkAllocateDescriptorSets(appBase->device, &descriptorSetAllocateInfo, &descriptorSet));

        VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo{};
        descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
        descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
        descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAS.accelerationStructure;

        VkWriteDescriptorSet accelerationStructureWrite{};
        accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // The specialized acceleration structure descriptor has to be chained
        accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
        accelerationStructureWrite.dstSet = descriptorSet;
        accelerationStructureWrite.dstBinding = 0;
        accelerationStructureWrite.descriptorCount = 1;
        accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

        VkDescriptorImageInfo storageImageDescriptor{};
        storageImageDescriptor.imageView = storageImage.view;
        storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkDescriptorBufferInfo vertexBufferDescriptor{};
        vertexBufferDescriptor.buffer = model->vertices.buffer;
        vertexBufferDescriptor.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo indexBufferDescriptor{};
        indexBufferDescriptor.buffer = model->indices.buffer;
        indexBufferDescriptor.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet resultImageWrite = SkInit::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor);
        VkWriteDescriptorSet uniformBufferWrite = SkInit::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &uniformBufferRT.descriptor);
        VkWriteDescriptorSet vertexBufferWrite = SkInit::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, &vertexBufferDescriptor);
        VkWriteDescriptorSet indexBufferWrite = SkInit::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, &indexBufferDescriptor);

        std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            accelerationStructureWrite,
            resultImageWrite,
            uniformBufferWrite,
            vertexBufferWrite,
            indexBufferWrite};
        vkUpdateDescriptorSets(appBase->device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
    }
    void BuildCommandBuffers()
    {
        rayCmdBuffers.resize(appBase->frameBuffers.size());
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = appBase->cmdPool;
        allocateInfo.commandBufferCount = (uint32_t)rayCmdBuffers.size();
        vkAllocateCommandBuffers(appBase->device, &allocateInfo, rayCmdBuffers.data());

        VkCommandBufferBeginInfo cmdBufInfo = SkInit::commandBufferBeginInfo();

        for (int32_t i = 0; i < rayCmdBuffers.size(); ++i)
        {
            VK_CHECK_RESULT(vkBeginCommandBuffer(rayCmdBuffers[i], &cmdBufInfo));

            /*
				Dispatch the ray tracing commands
			*/
            vkCmdBindPipeline(rayCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline);
            vkCmdBindDescriptorSets(rayCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipelineLayout, 0, 1, &descriptorSet, 0, 0);

            // Calculate shader binding offsets, which is pretty straight forward in our example
            VkDeviceSize bindingOffsetRayGenShader = rayTracingProperties.shaderGroupHandleSize * INDEX_RAYGEN;
            VkDeviceSize bindingOffsetMissShader = rayTracingProperties.shaderGroupHandleSize * INDEX_MISS;
            VkDeviceSize bindingOffsetHitShader = rayTracingProperties.shaderGroupHandleSize * INDEX_CLOSEST_HIT;
            VkDeviceSize bindingStride = rayTracingProperties.shaderGroupHandleSize;

            vkCmdTraceRaysNV(rayCmdBuffers[i],
                             shaderBindingTable.buffer, bindingOffsetRayGenShader,
                             shaderBindingTable.buffer, bindingOffsetMissShader, bindingStride,
                             shaderBindingTable.buffer, bindingOffsetHitShader, bindingStride,
                             VK_NULL_HANDLE, 0, 0,
                             appBase->width, appBase->height, 1);
            VK_CHECK_RESULT(vkEndCommandBuffer(rayCmdBuffers[i]));
        }
    }
    void Draw(uint32_t imageIndex)
    {     
        vkWaitForFences(appBase->device, 1, &(appBase->waitFences[imageIndex]), VK_TRUE, UINT64_MAX);
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(rayCmdBuffers[imageIndex]);
        VK_CHECK_RESULT(vkQueueSubmit(appBase->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    }
};