#pragma once
#include "SkBase.h"

class SkModel
{
private:
    
    SkBase *appBase;
public:
    std::vector<float> verticesData;
    std::vector<uint32_t> indicesData;
    bool useIndices = true;
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
        // uint32_t count;
    } indices;
    void Init(SkBase *initBase)
    {
        fprintf(stderr, "SkModel::Init...\n");

        appBase = initBase;
        verticesData.clear();
        indicesData.clear();
        // cmd = initCmd;
    }
    uint32_t GetVertexBufferSize()
    {
        return static_cast<uint32_t>(verticesData.size() * sizeof(float));
    }
    uint32_t GetIndexBufferSize()
    {
        return static_cast<uint32_t>(indicesData.size() * sizeof(uint32_t));
    }
    void LoadVerticesData(void *src, size_t f_size)
    {
        verticesData.resize(f_size);
        memcpy(verticesData.data(), src, sizeof(float) * verticesData.size());
    }
    void LoadIndicesData(void *src, size_t f_size)
    {
        indicesData.resize(f_size);
        memcpy(indicesData.data(), src, sizeof(uint32_t) * indicesData.size());
    }
    
    void CmdDraw(VkCommandBuffer cmdBuf)
    {
        VkDeviceSize offsets[1] = {0};
        if (useIndices)
        {
            vkCmdBindVertexBuffers(cmdBuf, 0, 1, &vertices.buffer, offsets);
            vkCmdBindIndexBuffer(cmdBuf, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmdBuf, (uint32_t)indicesData.size(), 1, 0, 0, 0);
        }
        else
        {
            if (vertices.buffer != VK_NULL_HANDLE)
            {

                vkCmdBindVertexBuffers(cmdBuf, 0, 1, &vertices.buffer, offsets);
            }
            vkCmdDraw(cmdBuf, (uint32_t)verticesData.size(), 1, 0, 0);
        }
    }
    void CleanUp()
    {
        fprintf(stderr,"SkModel::CleanUp...\n");
        
        vkFreeMemory(appBase->device, indices.memory, nullptr);
        vkDestroyBuffer(appBase->device, indices.buffer, nullptr);
        vkFreeMemory(appBase->device, vertices.memory, nullptr);
        vkDestroyBuffer(appBase->device, vertices.buffer, nullptr);
    }
    SkModel(/* args */){}
    ~SkModel(){}
};

