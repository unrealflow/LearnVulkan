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
        VERTEX_COMPONENT_DUMMY_VEC4 = 0x7
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
    struct ModelPart
    {
        uint32_t vertexBase;
        uint32_t vertexCount;
        uint32_t indexBase;
        uint32_t indexCount;
    };
    std::vector<ModelPart> parts;
    struct Dimension
    {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
        glm::vec3 size;
    } dim;

public:
    SkMesh mesh;
    // SkTexture texture;
    struct Texture
    {
        SkTexture *id;
        std::string type;
        std::string path;
    };
    std::vector<Texture> textures;
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
        layout = {{VERTEX_COMPONENT_POSITION,
                   VERTEX_COMPONENT_NORMAL,
                   VERTEX_COMPONENT_UV}};
        RebuildInputDescription();
    }
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
            case VERTEX_COMPONENT_POSITION:
                inputAttributes[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                _offset += sizeof(float) * 3;
                break;
            case VERTEX_COMPONENT_UV:
                inputAttributes[i].format = VK_FORMAT_R32G32_SFLOAT;
                _offset += sizeof(float) * 2;
                break;
            default:
                throw std::runtime_error("Components Error!");
                break;
            }
        }
    }
    void ImportModel(const std::string &path, ModelCreateInfo *createInfo = nullptr, VertexLayout *_layout = nullptr)
    {
        mesh.Init(appBase);
        Assimp::Importer Importer;
        const aiScene *pScene;
        pScene = Importer.ReadFile(path.c_str(), defaultFlags);
        if (!pScene)
        {
            std::string error = Importer.GetErrorString();
            throw std::runtime_error("Can't load " + path + "! " + error);
        }
        directory = path.substr(0, path.find_last_of('/'));
        if (pScene)
        {
            parts.clear();
            parts.resize(pScene->mNumMeshes);

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
            mesh.vertices.stride = layout.stride();
            for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
            {
                const aiMesh *paiMesh = pScene->mMeshes[i];

                parts[i] = {};
                parts[i].vertexBase = vertexCount;
                parts[i].indexBase = indexCount;

                vertexCount += pScene->mMeshes[i]->mNumVertices;

                aiColor3D pColor(0.f, 0.f, 0.f);
                aiMaterial *material = pScene->mMaterials[paiMesh->mMaterialIndex];
                material->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);

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
                            mesh.verticesData.push_back(pPos->x * scale.x + center.x);
                            mesh.verticesData.push_back(-pPos->y * scale.y + center.y);
                            mesh.verticesData.push_back(pPos->z * scale.z + center.z);
                            break;
                        case VERTEX_COMPONENT_NORMAL:
                            mesh.verticesData.push_back(pNormal->x);
                            mesh.verticesData.push_back(-pNormal->y);
                            mesh.verticesData.push_back(pNormal->z);
                            break;
                        case VERTEX_COMPONENT_UV:
                            mesh.verticesData.push_back(pTexCoord->x * uvscale.s);
                            mesh.verticesData.push_back(pTexCoord->y * uvscale.t);
                            break;
                        case VERTEX_COMPONENT_COLOR:
                            mesh.verticesData.push_back(pColor.r);
                            mesh.verticesData.push_back(pColor.g);
                            mesh.verticesData.push_back(pColor.b);
                            break;
                        case VERTEX_COMPONENT_TANGENT:
                            mesh.verticesData.push_back(pTangent->x);
                            mesh.verticesData.push_back(pTangent->y);
                            mesh.verticesData.push_back(pTangent->z);
                            break;
                        case VERTEX_COMPONENT_BITANGENT:
                            mesh.verticesData.push_back(pBiTangent->x);
                            mesh.verticesData.push_back(pBiTangent->y);
                            mesh.verticesData.push_back(pBiTangent->z);
                            break;
                        // Dummy components for padding
                        case VERTEX_COMPONENT_DUMMY_FLOAT:
                            mesh.verticesData.push_back(0.0f);
                            break;
                        case VERTEX_COMPONENT_DUMMY_VEC4:
                            mesh.verticesData.push_back(0.0f);
                            mesh.verticesData.push_back(0.0f);
                            mesh.verticesData.push_back(0.0f);
                            mesh.verticesData.push_back(0.0f);
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

                parts[i].vertexCount = paiMesh->mNumVertices;

                uint32_t indexBase = static_cast<uint32_t>(mesh.indicesData.size());
                for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
                {
                    const aiFace &Face = paiMesh->mFaces[j];
                    if (Face.mNumIndices != 3)
                        continue;
                    mesh.indicesData.push_back(indexBase + Face.mIndices[0]);
                    mesh.indicesData.push_back(indexBase + Face.mIndices[1]);
                    mesh.indicesData.push_back(indexBase + Face.mIndices[2]);
                    parts[i].indexCount += 3;
                    indexCount += 3;
                }

                diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            }
        }
    }

    std::string directory;
    std::vector<Texture> diffuseMaps;
    std::vector<Texture> specularMaps;
    std::vector<Texture> normalMaps;
    std::vector<Texture> heightMaps;
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> _textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures.size(); j++)
            {
                if (std::strcmp(textures[j].path.data(), str.C_Str()) == 0)
                {
                    _textures.push_back(textures[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            { // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = new SkTexture();
                // directory + '/' +
                texture.id->Init(str.C_Str());
                texture.type = typeName;
                texture.path = str.C_Str();
                _textures.push_back(texture);
                textures.push_back(texture); // store it as texture loaded for entire mesh, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return _textures;
    }

    void Build(SkMemory *mem)
    {
        BuildMesh(&mesh);
        for (size_t i = 0; i < textures.size(); i++)
        {
            BuildTexture(textures[i].id);
        }
    }
    void BuildTexture(SkTexture *tex, bool useStaging = false)
    {
        if (useStaging)
        {
            mem->CreateLocalImage(tex->data, tex->GetExtent3D(), VK_IMAGE_USAGE_SAMPLED_BIT, &tex->image.image, &tex->image.memory, &tex->layout);
        }
        else
        {
            mem->CreateImage(tex->data, tex->GetExtent3D(), VK_IMAGE_USAGE_SAMPLED_BIT, &tex->image.image, &tex->image.memory, &tex->layout);
        }
        mem->CreateImageView(tex->image.image, tex->image.format, VK_IMAGE_ASPECT_COLOR_BIT, &tex->image.view);
        mem->SetupDescriptor(&tex->image, tex->layout);
    }
    void BuildMesh(SkMesh *mesh, bool useStaging = true)
    {

        if (useStaging)
        {
            mem->CreateLocalBuffer(mesh->verticesData.data(),
                                   mesh->GetVertexBufferSize(),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   &mesh->vertices.buffer,
                                   &mesh->vertices.memory);
            mem->CreateLocalBuffer(mesh->indicesData.data(),
                                   mesh->GetIndexBufferSize(),
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   &mesh->indices.buffer,
                                   &mesh->indices.memory);
        }
        else
        {
            mem->CreateBuffer(mesh->verticesData.data(),
                              mesh->GetVertexBufferSize(),
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              &mesh->vertices.buffer,
                              &mesh->vertices.memory);
            mem->CreateBuffer(mesh->indicesData.data(),
                              mesh->GetIndexBufferSize(),
                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              &mesh->indices.buffer,
                              &mesh->indices.memory);
        }
    }
    VkDescriptorImageInfo *GetTexDes(uint32_t i)
    {
        if (i >= textures.size())
        {
            return nullptr;
        }
        return &(textures[i].id->image.descriptor);
    }
    void UsePipeline(SkGraphicsPipeline *pipeline)
    {
        pipeline->meshes.push_back(&mesh);
    }
    void CleanUp()
    {
        mesh.CleanUp();
        for (size_t i = 0; i < textures.size(); i++)
        {
            mem->FreeImage(&(textures[i].id->image));
            delete textures[i].id;
            textures[i].id = nullptr;
        }
    }
};
