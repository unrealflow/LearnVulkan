#pragma once
#include "SkBase.h"

class SkMesh
{
private:
    SkBase *appBase;

public:
    std::vector<float> verticesData;
    std::vector<uint32_t> indicesData;
    glm::mat4 model;
    bool useIndices = true;
    struct
    {
        VkDeviceMemory memory=VK_NULL_HANDLE; // Handle to the device memory for this buffer
        VkBuffer buffer=VK_NULL_HANDLE;       // Handle to the Vulkan buffer object that the memory is bound to
        uint32_t stride;
    } vertices;
    // Index buffer
    struct
    {
        VkDeviceMemory memory=VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        // uint32_t count;
    } indices;
    void Init(SkBase *initBase)
    {
        fprintf(stderr, "SkMesh::Init...\n");

        appBase = initBase;
        verticesData.clear();
        indicesData.clear();
        // cmd = initCmd;
    }
    uint32_t GetVertexCount()
    {
        return static_cast<uint32_t>(verticesData.size());
    }
    uint32_t GetIndexCount()
    {
        return static_cast<uint32_t>(indicesData.size());
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
    void CreatePlane()
    {
        struct Vertex
        {
            glm::vec3 Position;
            glm::vec3 Normal;
            glm::vec2 UV;
        };
        std::vector<Vertex> _verticesData =
            {
                {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            };
        indicesData = {0, 1, 2, 0, 2, 3};
        LoadVerticesData(verticesData.data(), verticesData.size() * sizeof(Vertex));
        vertices.stride=sizeof(Vertex);
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
        fprintf(stderr, "SkMesh::CleanUp...\n");

        vkFreeMemory(appBase->device, indices.memory, nullptr);
        vkDestroyBuffer(appBase->device, indices.buffer, nullptr);
        vkFreeMemory(appBase->device, vertices.memory, nullptr);
        vkDestroyBuffer(appBase->device, vertices.buffer, nullptr);
    }
    SkMesh(/* args */) {}
    ~SkMesh() {}
};
