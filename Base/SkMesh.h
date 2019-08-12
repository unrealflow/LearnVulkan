#pragma once
#include "SkBase.h"
#include "SkMaterial.h"

//网格部分，记录顶点和索引信息
class SkMesh
{
private:
    SkMemory *mem;
    SkMatSet *matSet = nullptr;
    uint32_t matIndex;

public:
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet desSet;
    std::vector<float> verticesData;
    std::vector<uint32_t> indicesData;
    glm::mat4 transform;
     uint32_t stride;
    // SkMaterial mat;
    bool useIndices = true;

    SkBuffer vertices;
    SkBuffer indices;
    void Init(SkMemory *initMem)
    {
        fprintf(stderr, "SkMesh::Init...\n");
        mem = initMem;
        // mat.Init(mem);
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
                                   &vertices);
            mem->CreateLocalBuffer(this->indicesData.data(),
                                   this->GetIndexBufferSize(),
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   &indices);
        }
        else
        {
            mem->CreateBuffer(this->verticesData.data(),
                              this->GetVertexBufferSize(),
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              &vertices);
            mem->CreateBuffer(this->indicesData.data(),
                              this->GetIndexBufferSize(),
                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              &indices);
        }
        mem->SetupDescriptor(&vertices);
        mem->SetupDescriptor(&indices);
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
        mem->FreeBuffer(&vertices);
        mem->FreeBuffer(&indices);
    }
    void AddRayBindings(std::vector<VkDescriptorSetLayoutBinding> &bindings, uint32_t index = 0)
    {
        bindings.emplace_back(
            SkInit::descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
                LOC::VERTEX + index * LOC::STRIDE));
        bindings.emplace_back(
            SkInit::descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
                LOC::INDEX + index * LOC::STRIDE));
        SkMaterial::AddRayMatBinding(bindings, index);
    }

    void SetWriteDes(std::vector<VkWriteDescriptorSet> &writeSets,
                     VkDescriptorSet desSet,
                     uint32_t index = 0)
    {
        writeSets.emplace_back(
            SkInit::writeDescriptorSet(
                desSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                LOC::VERTEX+ index * LOC::STRIDE,
                &vertices.descriptor));
        writeSets.emplace_back(
            SkInit::writeDescriptorSet(
                desSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                LOC::INDEX+ index * LOC::STRIDE,
                &indices.descriptor));
        GetMat()->SetWriteDes(writeSets,desSet,index);
    }
    SkMesh(/* args */) {}
    ~SkMesh() {}
};
