#pragma once
#include "SkBase.h"
#include "SkMaterial.h"
class SkMesh
{
private:
    SkBase *appBase;
    SkMemory *mem;

public:
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet desSet;
    std::vector<float> verticesData;
    std::vector<uint32_t> indicesData;
    uint32_t matIndex;
    glm::mat4 transform;
    SkMaterial mat;
    bool useIndices = true;
    struct
    {
        VkDeviceMemory memory = VK_NULL_HANDLE; // Handle to the device memory for this buffer
        VkBuffer buffer = VK_NULL_HANDLE;       // Handle to the Vulkan buffer object that the memory is bound to
        uint32_t stride;
    } vertices;
    // Index buffer
    struct
    {
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        // uint32_t count;
    } indices;
    void Init(SkBase *initBase, SkMemory *initMem)
    {
        fprintf(stderr, "SkMesh::Init...\n");

        appBase = initBase;
        mem = initMem;
        mat.Init(mem);
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
    void Build(bool useStaging = true)
    {

        if (useStaging)
        {
            mem->CreateLocalBuffer(this->verticesData.data(),
                                   this->GetVertexBufferSize(),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   &this->vertices.buffer,
                                   &this->vertices.memory);
            mem->CreateLocalBuffer(this->indicesData.data(),
                                   this->GetIndexBufferSize(),
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   &this->indices.buffer,
                                   &this->indices.memory);
        }
        else
        {
            mem->CreateBuffer(this->verticesData.data(),
                              this->GetVertexBufferSize(),
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              &this->vertices.buffer,
                              &this->vertices.memory);
            mem->CreateBuffer(this->indicesData.data(),
                              this->GetIndexBufferSize(),
                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              &this->indices.buffer,
                              &this->indices.memory);
        }
        this->mat.Build();
    }

    void CmdDraw(VkCommandBuffer cmdBuf)
    {
        VkDeviceSize offsets[1] = {0};
        vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1, &this->desSet, 0, nullptr);
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
        mat.CleanUp();
        vkFreeMemory(appBase->device, indices.memory, nullptr);
        vkDestroyBuffer(appBase->device, indices.buffer, nullptr);
        vkFreeMemory(appBase->device, vertices.memory, nullptr);
        vkDestroyBuffer(appBase->device, vertices.buffer, nullptr);
    }
    SkMesh(/* args */) {}
    ~SkMesh() {}
};
