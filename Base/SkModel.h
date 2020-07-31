#pragma once
#include "SkBase.h"
#include "SkAgent.h"
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
    SkAgent *agent;
    static const int defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;

    struct Dimension
    {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
        glm::vec3 size;
    } dim;
    std::vector<VkDescriptorImageInfo> totalTexInfos = {};
    std::vector<VkDescriptorBufferInfo> totalBufInfos={};
    const int MAX_MAT_COUNT=10;
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
    void Init(SkAgent *initAgent)
    {
        agent = initAgent;
        matSet.Init(agent);
        layout = {{
            VERTEX_COMPONENT_POSITION,
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
    void ImportModel(std::string path, ModelCreateInfo *createInfo = nullptr, VertexLayout *_layout = nullptr)
    {
        // mesh.Init(appBase);
        Assimp::Importer Importer;
        const aiScene *pScene;
        path = DataDir() + path;
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
                matSet.AddMat(pScene->mMaterials[i], directory);
            }
            for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
            {
                const aiMesh *paiMesh = pScene->mMeshes[i];
                meshes[i].Init(agent);
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
    void ImportScene(BScene *scene)
    {
        if (nullptr == scene)
        {
            throw std::exception("No Scene Data");
        }
        this->layout =
            {{
                VERTEX_COMPONENT_POSITION,
                VERTEX_COMPONENT_NORMAL,
                VERTEX_COMPONENT_UV,
            }};
        RebuildInputDescription();
        meshes.clear();
        meshes.resize(scene->nums);
        vertexCount = 0;
        indexCount = 0;
        for (uint32_t i = 0; i < scene->nums; i++)
        {
            glm::vec3 pos = scene->meshes[i].T->Position;
            glm::vec3 rot = scene->meshes[i].T->Rotation;
            glm::vec3 scale = scene->meshes[i].T->Scale;
            meshes[i].Init(agent);
            vertexCount += scene->meshes[i].Vc;
            indexCount += scene->meshes[i].Ic;
            meshes[i].stride = layout.stride();
            meshes[i].verticesData.resize(scene->meshes[i].Vc);
            memcpy(meshes[i].verticesData.data(),
                   scene->meshes[i].V,
                   sizeof(float) * scene->meshes[i].Vc);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos);
            model = glm::scale(model, scale);
            model = glm::rotate(model, rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, rot.x, glm::vec3(1.0f, 0.0f, 0.0f));

            for (uint32_t p = 0; p < meshes[i].verticesData.size(); p += 8)
            {
                glm::vec4 v_pos = glm::vec4(meshes[i].verticesData[p],
                                            meshes[i].verticesData[p + 1],
                                            meshes[i].verticesData[p + 2],
                                            1.0f);

                glm::vec4 v_normal = glm::vec4(meshes[i].verticesData[p + 3],
                                               meshes[i].verticesData[p + 4],
                                               meshes[i].verticesData[p + 5],
                                               0.0f);
                v_pos = model * v_pos;
                v_pos /= v_pos.w;
                v_normal = model * v_normal;

                meshes[i].verticesData[p + 0] = v_pos.x;
                meshes[i].verticesData[p + 1] = -v_pos.z;
                meshes[i].verticesData[p + 2] = -v_pos.y;
                meshes[i].verticesData[p + 3] = v_normal.x;
                meshes[i].verticesData[p + 4] = -v_normal.z;
                meshes[i].verticesData[p + 5] = -v_normal.y;
            }

            meshes[i].indicesData.resize(scene->meshes[i].Ic);
            memcpy(meshes[i].indicesData.data(),
                   scene->meshes[i].I,
                   sizeof(uint32_t) * scene->meshes[i].Ic);
            SkMaterial mat;
            mat.Init(agent);
            mat.mat.baseColor = scene->meshes[i].M->baseColor;
            mat.mat.metallic = scene->meshes[i].M->metallic;
            mat.mat.roughness = scene->meshes[i].M->roughness;
            mat.mat.index = float(i);
            matSet.AddMat(mat);
            meshes[i].SetMat(&matSet, i);
        }
    }
    //将模型数据加载至显存
    void Build()
    {
        for (size_t i = 0; i < meshes.size(); i++)
        {
            meshes[i].GetMat()->mat.index=float(i);
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
    void AddAllBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings)
    {
        uint32_t count=static_cast<uint32_t>(meshes.size());
        bindings.emplace_back(
            SkInit::descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_FRAGMENT_BIT, LOC::UNIFORM,count));
        bindings.emplace_back(
            SkInit::descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT, LOC::DIFFUSE,count));
    }
    void SetAllWirteDes(std::vector<VkWriteDescriptorSet> &writeSets, VkDescriptorSet desSet)
    {
        totalBufInfos.clear();
        totalTexInfos.clear();
        for (size_t i = 0; i < meshes.size(); i++)
        {
            totalBufInfos.push_back(meshes[i].GetMat()->matBuf.descriptor);
            totalTexInfos.push_back(meshes[i].GetMat()->diffuseMaps[0].id->image.descriptor);
        }

        writeSets.emplace_back(
            SkInit::writeDescriptorSet(
                desSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                LOC::UNIFORM, totalBufInfos.data(),
                static_cast<uint32_t>(totalBufInfos.size())));

        writeSets.emplace_back(
            SkInit::writeDescriptorSet(
                desSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                LOC::DIFFUSE, totalTexInfos.data(),
                static_cast<uint32_t>(totalTexInfos.size())));
    }
};
