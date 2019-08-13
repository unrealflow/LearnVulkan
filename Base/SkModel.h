#pragma once
#include "SkBase.h"
#include "SkMemory.h"
#include "SkMesh.h"
#include "SkTexture.h"
#include "SkGraphicsPipeline.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

//模型加载与解析
class SkModel
{
public:
    typedef enum Component
    {
        VERTEX_COMPONENT_POSITION = 0x0,
        VERTEX_COMPONENT_NORMAL = 0x1,
        VERTEX_COMPONENT_COLOR = 0x2,
        VERTEX_COMPONENT_UV = 0x3,
        VERTEX_COMPONENT_TANGENT = 0x4,
        VERTEX_COMPONENT_BITANGENT = 0x5,
        VERTEX_COMPONENT_DUMMY_FLOAT = 0x6,
        VERTEX_COMPONENT_DUMMY_VEC4 = 0x7,
        VERTEX_COMPONENT_MATINDEX = 0x8
    } Component;

    /** @brief Stores vertex layout components for mesh loading and Vulkan vertex input and atribute bindings  */
    struct VertexLayout
    {
    public:
        /** @brief Components used to generate vertices from */
        std::vector<Component> components;
        VertexLayout()
        {
        }
        VertexLayout(std::vector<Component> components)
        {
            this->components = std::move(components);
        }

        uint32_t stride()
        {
            uint32_t res = 0;
            for (auto &component : components)
            {
                switch (component)
                {
                case VERTEX_COMPONENT_UV:
                    res += 2 * sizeof(float);
                    break;
                case VERTEX_COMPONENT_MATINDEX:
                    res += sizeof(float);
                    break;
                case VERTEX_COMPONENT_DUMMY_FLOAT:
                    res += sizeof(float);
                    break;
                case VERTEX_COMPONENT_DUMMY_VEC4:
                    res += 4 * sizeof(float);
                    break;
                default:
                    // All components except the ones listed above are made up of 3 floats
                    res += 3 * sizeof(float);
                }
            }
            return res;
        }
    };
    struct ModelCreateInfo
    {
        glm::vec3 center;
        glm::vec3 scale;
        glm::vec2 uvscale;
        VkMemoryPropertyFlags memoryPropertyFlags = 0;

        ModelCreateInfo() : center(glm::vec3(0.0f)), scale(glm::vec3(1.0f)), uvscale(glm::vec2(1.0f)){};

        ModelCreateInfo(glm::vec3 scale, glm::vec2 uvscale, glm::vec3 center)
        {
            this->center = center;
            this->scale = scale;
            this->uvscale = uvscale;
        }

        ModelCreateInfo(float scale, float uvscale, float center)
        {
            this->center = glm::vec3(center);
            this->scale = glm::vec3(scale);
            this->uvscale = glm::vec2(uvscale);
        }
    };

private:
    SkBase *appBase;
    SkMemory *mem;
    static const int defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;

    struct Dimension
    {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
        glm::vec3 size;
    } dim;

public:
    std::vector<SkMesh> meshes;
    SkMatSet matSet;
    // SkTexture texture;
    uint32_t indexCount = 0;
    uint32_t vertexCount = 0;
    std::vector<VkVertexInputBindingDescription> inputBindings;
    std::vector<VkVertexInputAttributeDescription> inputAttributes;
    VertexLayout layout;
    SkModel(/* args */) {}
    ~SkModel() {}
    void Init(SkBase *initBase, SkMemory *initMem)
    {
        appBase = initBase;
        mem = initMem;
        matSet.Init(mem);
        layout = {{VERTEX_COMPONENT_POSITION,
                   VERTEX_COMPONENT_NORMAL,
                   VERTEX_COMPONENT_UV,
                   }};
        RebuildInputDescription();
    }
    //根据设置生成InputBindingDescription
    void RebuildInputDescription()
    {
        inputBindings.resize(1);
        inputBindings[0].binding = 0;
        inputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindings[0].stride = layout.stride();

        inputAttributes.resize(layout.components.size());
        uint32_t _offset = 0;
        for (size_t i = 0; i < layout.components.size(); i++)
        {
            inputAttributes[i].binding = 0;
            inputAttributes[i].location = static_cast<uint32_t>(i);
            inputAttributes[i].offset = _offset;
            switch (layout.components[i])
            {
            case VERTEX_COMPONENT_NORMAL:
            case VERTEX_COMPONENT_COLOR:
            case VERTEX_COMPONENT_POSITION:
                inputAttributes[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                _offset += sizeof(float) * 3;
                break;
            case VERTEX_COMPONENT_UV:
                inputAttributes[i].format = VK_FORMAT_R32G32_SFLOAT;
                _offset += sizeof(float) * 2;
                break;
            case VERTEX_COMPONENT_MATINDEX:
                inputAttributes[i].format = VK_FORMAT_R32_SFLOAT;
                _offset += sizeof(float);
                break;
            default:
                throw std::runtime_error("Components Error!");
                break;
            }
        }
    }
    //加载模型，导入数据
    void ImportModel(const std::string &path, ModelCreateInfo *createInfo = nullptr, VertexLayout *_layout = nullptr)
    {
        // mesh.Init(appBase);
        Assimp::Importer Importer;
        const aiScene *pScene;
        pScene = Importer.ReadFile(path.c_str(), defaultFlags);
        if (!pScene)
        {
            std::string error = Importer.GetErrorString();
            throw std::runtime_error("Can't load " + path + "! " + error);
        }
        std::string directory = path.substr(0, path.find_last_of('/'));
        if (pScene)
        {
            meshes.clear();
            meshes.resize(pScene->mNumMeshes);

            glm::vec3 scale(1.0f);
            glm::vec2 uvscale(1.0f);
            glm::vec3 center(0.0f);
            if (createInfo)
            {
                scale = createInfo->scale;
                uvscale = createInfo->uvscale;
                center = createInfo->center;
            }
            vertexCount = 0;
            indexCount = 0;
            if (_layout != nullptr)
            {
                this->layout = VertexLayout(*_layout);
                RebuildInputDescription();
            }
            for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
            {
                uint32_t index = matSet.AddMat(pScene->mMaterials[i]);
                assert(i == index);
            }
            for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
            {
                const aiMesh *paiMesh = pScene->mMeshes[i];
                meshes[i].Init(mem);
                meshes[i].stride = layout.stride();

                vertexCount += pScene->mMeshes[i]->mNumVertices;

                aiColor3D pColor(0.f, 0.f, 0.f);
                aiMaterial *material = pScene->mMaterials[paiMesh->mMaterialIndex];
                material->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);
                meshes[i].SetMat(&matSet, paiMesh->mMaterialIndex);
                // meshes[i].mat.LoadMaterial(material);

                const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

                for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
                {
                    const aiVector3D *pPos = &(paiMesh->mVertices[j]);
                    const aiVector3D *pNormal = &(paiMesh->mNormals[j]);
                    const aiVector3D *pTexCoord = (paiMesh->HasTextureCoords(0)) ? &(paiMesh->mTextureCoords[0][j]) : &Zero3D;
                    const aiVector3D *pTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mTangents[j]) : &Zero3D;
                    const aiVector3D *pBiTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mBitangents[j]) : &Zero3D;

                    for (auto &component : layout.components)
                    {
                        switch (component)
                        {
                        case VERTEX_COMPONENT_POSITION:
                            meshes[i].verticesData.push_back(pPos->x * scale.x + center.x);
                            meshes[i].verticesData.push_back(-pPos->y * scale.y + center.y);
                            meshes[i].verticesData.push_back(pPos->z * scale.z + center.z);
                            break;
                        case VERTEX_COMPONENT_NORMAL:
                            meshes[i].verticesData.push_back(pNormal->x);
                            meshes[i].verticesData.push_back(-pNormal->y);
                            meshes[i].verticesData.push_back(pNormal->z);
                            break;
                        case VERTEX_COMPONENT_UV:
                            meshes[i].verticesData.push_back(pTexCoord->x * uvscale.s);
                            meshes[i].verticesData.push_back(pTexCoord->y * uvscale.t);
                            break;
                        case VERTEX_COMPONENT_COLOR:
                            meshes[i].verticesData.push_back(pColor.r);
                            meshes[i].verticesData.push_back(pColor.g);
                            meshes[i].verticesData.push_back(pColor.b);
                            break;
                        case VERTEX_COMPONENT_TANGENT:
                            meshes[i].verticesData.push_back(pTangent->x);
                            meshes[i].verticesData.push_back(pTangent->y);
                            meshes[i].verticesData.push_back(pTangent->z);
                            break;
                        case VERTEX_COMPONENT_BITANGENT:
                            meshes[i].verticesData.push_back(pBiTangent->x);
                            meshes[i].verticesData.push_back(pBiTangent->y);
                            meshes[i].verticesData.push_back(pBiTangent->z);
                            break;
                        // Dummy components for padding
                        case VERTEX_COMPONENT_DUMMY_FLOAT:
                            meshes[i].verticesData.push_back(0.0f);
                            break;
                        case VERTEX_COMPONENT_DUMMY_VEC4:
                            meshes[i].verticesData.push_back(0.0f);
                            meshes[i].verticesData.push_back(0.0f);
                            meshes[i].verticesData.push_back(0.0f);
                            meshes[i].verticesData.push_back(0.0f);
                            break;
                        case VERTEX_COMPONENT_MATINDEX:
                            meshes[i].verticesData.push_back(static_cast<float>(paiMesh->mMaterialIndex));
                            break;
                        };
                    }
                    dim.max.x = fmax(pPos->x, dim.max.x);
                    dim.max.y = fmax(pPos->y, dim.max.y);
                    dim.max.z = fmax(pPos->z, dim.max.z);

                    dim.min.x = fmin(pPos->x, dim.min.x);
                    dim.min.y = fmin(pPos->y, dim.min.y);
                    dim.min.z = fmin(pPos->z, dim.min.z);
                }

                dim.size = dim.max - dim.min;

                for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
                {
                    const aiFace &Face = paiMesh->mFaces[j];
                    if (Face.mNumIndices != 3)
                        continue;
                    meshes[i].indicesData.push_back(Face.mIndices[0]);
                    meshes[i].indicesData.push_back(Face.mIndices[1]);
                    meshes[i].indicesData.push_back(Face.mIndices[2]);
                    indexCount += 3;
                }
            }
        }
    }
    //将模型数据加载至显存
    void Build()
    {
        for (size_t i = 0; i < meshes.size(); i++)
        {
            meshes[i].Build();
        }
        matSet.Build();
    }
    //使用指定管线绘制模型
    void UsePipeline(SkGraphicsPipeline *pipeline)
    {
        for (size_t i = 0; i < meshes.size(); i++)
        {
            pipeline->meshes.push_back(&meshes[i]);
        }
    }
    void CleanUp()
    {
        for (size_t i = 0; i < meshes.size(); i++)
        {
            meshes[i].CleanUp();
        }
        matSet.CleanUp();
    }
    //为每个mesh生成descriptorSet
    void SetupDescriptorSet(SkGraphicsPipeline *pipeline,
                            const std::vector<VkWriteDescriptorSet> &writeSets,
                            bool alloc = true)
    {
        for (size_t i = 0; i < meshes.size(); i++)
        {
            std::vector<VkWriteDescriptorSet> t_writeSets = writeSets;
            meshes[i].GetMat()->SetWriteDes(t_writeSets);
            pipeline->SetupDescriptorSet(&meshes[i], t_writeSets, alloc);
        }
    }
};
