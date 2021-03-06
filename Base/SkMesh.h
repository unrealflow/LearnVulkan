﻿#pragma once
#include "SkBase.h"
#include "SkMaterial.h"

//网格部分，记录顶点和索引信息
class SkMesh
{
private:
    SkAgent *agent;
    SkMatSet *matSet = nullptr;
    uint32_t matIndex;

public:
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet desSet = VK_NULL_HANDLE;
    std::vector<float> verticesData;
    std::vector<uint32_t> indicesData;
    glm::mat4 transform;
    uint32_t stride;
    // SkMaterial mat;
    bool useIndices = true;

    SkBuffer vertices;
    SkBuffer indices;
    void Init(SkAgent *initAgent)
    {
        agent = initAgent;
        verticesData.clear();
        indicesData.clear();
        // cmd = initCmd;
    }
    void SetMat(SkMatSet *_matSet, uint32_t _index)
    {
        matSet = _matSet;
        matIndex = _index;
    }
    SkMaterial *GetMat()
    {
        if (matSet == nullptr)
        {
            fprintf(stderr, "ERROR: Mat not Set!...\n");
            return nullptr;
        }
        return matSet->GetMat(matIndex);
    }
    uint32_t GetVertexCount()
    {
        return static_cast<uint32_t>(verticesData.size() * sizeof(float) / stride);
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
            agent->CreateLocalBuffer(this->verticesData.data(),
                                   this->GetVertexBufferSize(),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   &vertices);
            agent->CreateLocalBuffer(this->indicesData.data(),
                                   this->GetIndexBufferSize(),
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   &indices);
        }
        else
        {
            agent->CreateBuffer(this->verticesData.data(),
                              this->GetVertexBufferSize(),
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              &vertices);
            agent->CreateBuffer(this->indicesData.data(),
                              this->GetIndexBufferSize(),
                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              &indices);
        }
        agent->SetupDescriptor(&vertices);
        agent->SetupDescriptor(&indices);
        // this->mat.Build();
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
        // mat.CleanUp();
        agent->FreeBuffer(&vertices);
        agent->FreeBuffer(&indices);
    }
    void AddRayBindings(std::vector<VkDescriptorSetLayoutBinding> &bindings, uint32_t index = 0)
    {
        SkMaterial::AddRayMatBinding(bindings, index);
    }
    //将mesh的顶点、索引、材质信息写入desSet
    //若desSet==0则写入mesh自身的desSet
    void SetWriteDes(std::vector<VkWriteDescriptorSet> &writeSets,
                     VkDescriptorSet desSet = 0,
                     uint32_t index = 0)
    {
        if (desSet == 0)
        {
            desSet = this->desSet;
        }
        GetMat()->SetWriteDes(writeSets, desSet, index);
    }
    SkMesh(/* args */) {}
    ~SkMesh() {}
};
