#pragma once
#include "SkBase.h"

class SkModel
{
private:
    SkBase *appBase;
    SkCmd *cmd;

    std::vector<float> verticesData;
    std::vector<uint32_t> indicesData;
    void CreateBuffer(bool useStaging)
    {
        if (useStaging)
        {
            cmd->CreateLocalBuffer(verticesData.data(), GetVertexBufferSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &vertices.buffer, &vertices.memory);
            cmd->CreateLocalBuffer(indicesData.data(), GetIndexBufferSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &indices.buffer, &indices.memory);
        }
        else
        {
            cmd->CreateBuffer(verticesData.data(), GetVertexBufferSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &vertices.buffer, &vertices.memory);
            cmd->CreateBuffer(indicesData.data(), GetIndexBufferSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &indices.buffer, &indices.memory);
        }
    }

public:
    struct
    {
        VkDeviceMemory memory; // Handle to the device memory for this buffer
        VkBuffer buffer;       // Handle to the Vulkan buffer object that the memory is bound to
    } vertices;
    // Index buffer
    struct
    {
        VkDeviceMemory memory;
        VkBuffer buffer;
        uint32_t count;
    } indices;

    uint32_t GetVertexBufferSize()
    {
        return static_cast<uint32_t>(verticesData.size() * sizeof(float));
    }
    uint32_t GetIndexBufferSize()
    {
        return static_cast<uint32_t>(indicesData.size() * sizeof(uint32_t));
    }
    SkModel(/* args */);
    ~SkModel();
};

SkModel::SkModel(/* args */)
{
}

SkModel::~SkModel()
{
}
