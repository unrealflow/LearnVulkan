#pragma once
#include "SkCommom.h"

namespace SkInit
{

inline VkMemoryAllocateInfo memoryAllocateinfo(
    VkDeviceSize allocationSize,
    uint32_t memoryTypeIndex)
{
    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = allocationSize;
    memAllocInfo.memoryTypeIndex = memoryTypeIndex;
    // memAllocInfo.pNext = pNext;
    return memAllocInfo;
}

inline VkMappedMemoryRange mappedMemoryRange()
{
    VkMappedMemoryRange mappedMemoryRange{};
    mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    return mappedMemoryRange;
}

inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
    VkCommandPool commandPool,
    VkCommandBufferLevel level,
    uint32_t bufferCount)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = level;
    commandBufferAllocateInfo.commandBufferCount = bufferCount;
    return commandBufferAllocateInfo;
}
inline VkCommandPoolCreateInfo commandPoolCreateInfo()
{
    VkCommandPoolCreateInfo cmdPoolCreateInfo{};
    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    return cmdPoolCreateInfo;
}

inline VkCommandBufferBeginInfo commandBufferBeginInfo()
{
    VkCommandBufferBeginInfo cmdBufferBeginInfo{};
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    return cmdBufferBeginInfo;
}

inline VkCommandBufferInheritanceInfo commandBufferInheritanceInfo()
{
    VkCommandBufferInheritanceInfo cmdBufferInheritanceInfo{};
    cmdBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    return cmdBufferInheritanceInfo;
}

inline VkRenderPassBeginInfo renderPassBeginInfo()
{
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    return renderPassBeginInfo;
}

inline VkRenderPassCreateInfo renderPassCreateInfo()
{
    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    return renderPassCreateInfo;
}

//初始化一个图像内存屏障
inline VkImageMemoryBarrier imageMemoryBarrier()
{
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    return imageMemoryBarrier;
}
//初始化一个缓冲内存屏障
inline VkBufferMemoryBarrier bufferMemoryBarrier()
{
    VkBufferMemoryBarrier bufferMemoryBarrier{};
    bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    return bufferMemoryBarrier;
}
inline VkMemoryBarrier memoryBarrier()
{
    VkMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    return memoryBarrier;
}
inline VkImageCreateInfo imageCreateInfo()
{
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    return imageCreateInfo;
}
inline VkSamplerCreateInfo samplerCreateInfo()
{
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    return samplerCreateInfo;
}
inline VkImageViewCreateInfo imageViewCreateInfo()
{
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    return imageViewCreateInfo;
}
inline VkFramebufferCreateInfo framebufferCreateInfo()
{
    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    return framebufferCreateInfo;
}
inline VkSemaphoreCreateInfo semaphoreCreateInfo()
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    return semaphoreCreateInfo;
}
inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0)
{
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = flags;
    return fenceCreateInfo;
}
inline VkEventCreateInfo eventCreateInfo()
{
    VkEventCreateInfo eventCreateInfo{};
    eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    return eventCreateInfo;
}
inline VkSubmitInfo submitInfo()
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    return submitInfo;
}
inline VkViewport viewport(
    float width,
    float height,
    float minDepth,
    float maxDepth)
{
    VkViewport viewport{};
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    return viewport;
}

inline VkRect2D rect2D(
    int32_t width,
    int32_t height,
    int32_t offsetX,
    int32_t offsetY)
{
    VkRect2D rect2D{};
    rect2D.extent.width = width;
    rect2D.extent.height = height;
    rect2D.offset.x = offsetX;
    rect2D.offset.y = offsetY;
    return rect2D;
}
inline VkBufferCreateInfo bufferCreateInfo()
{
    VkBufferCreateInfo bufCreateInfo{};
    bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    return bufCreateInfo;
}
inline VkBufferCreateInfo bufferCreateInfo(
    VkBufferUsageFlags usage,
    VkDeviceSize size)
{
    VkBufferCreateInfo bufCreateInfo{};
    bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufCreateInfo.usage = usage;
    bufCreateInfo.size = size;
    return bufCreateInfo;
}
inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
    uint32_t poolSizeCount,
    VkDescriptorPoolSize *pPoolSizes,
    uint32_t maxSets)
{
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = poolSizeCount;
    descriptorPoolCreateInfo.pPoolSizes = pPoolSizes;
    descriptorPoolCreateInfo.maxSets = maxSets;
    return descriptorPoolCreateInfo;
}
inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
    const std::vector<VkDescriptorPoolSize> &poolSizes,
    uint32_t maxSets)
{
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
    descriptorPoolCreateInfo.maxSets = maxSets;
    return descriptorPoolCreateInfo;
}
inline VkDescriptorPoolSize descriptorPoolSize(
    VkDescriptorType type,
    uint32_t descriptorCount)
{
    VkDescriptorPoolSize descriptorPoolSize{};
    descriptorPoolSize.type = type;
    descriptorPoolSize.descriptorCount = descriptorCount;
    return descriptorPoolSize;
}
inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
    VkDescriptorType type,
    VkShaderStageFlags stageFlags,
    uint32_t binding,
    uint32_t descriptorCount = 1)
{
    VkDescriptorSetLayoutBinding setLayoutBinding{};
    setLayoutBinding.descriptorType = type;
    setLayoutBinding.stageFlags = stageFlags;
    setLayoutBinding.binding = binding;
    setLayoutBinding.descriptorCount = descriptorCount;
    return setLayoutBinding;
}
inline VkDescriptorSetLayoutCreateInfo descirptorSetLayoutCreateInfo(
    const VkDescriptorSetLayoutBinding *pBindings,
    uint32_t bindingCount)
{
    VkDescriptorSetLayoutCreateInfo descirptorSetLayoutCreateInfo{};
    descirptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descirptorSetLayoutCreateInfo.pBindings = pBindings;
    descirptorSetLayoutCreateInfo.bindingCount = bindingCount;
    return descirptorSetLayoutCreateInfo;
}
inline VkDescriptorSetLayoutCreateInfo descirptorSetLayoutCreateInfo(
    const std::vector<VkDescriptorSetLayoutBinding> &bindings)
{
    VkDescriptorSetLayoutCreateInfo descirptorSetLayoutCreateInfo{};
    descirptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descirptorSetLayoutCreateInfo.pBindings = bindings.data();
    descirptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    return descirptorSetLayoutCreateInfo;
}
inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
    const VkDescriptorSetLayout *pSetLayouts,
    uint32_t setLayoutCount = 1)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
    pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
    return pipelineLayoutCreateInfo;
}
inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(uint32_t setLayoutCount = 1)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
    return pipelineLayoutCreateInfo;
}
inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(VkDescriptorPool descriptorPool, const VkDescriptorSetLayout *pSetLayouts, uint32_t descriptorSetCount)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
    descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
    return descriptorSetAllocateInfo;
}

} // namespace VkInitializers