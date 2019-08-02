#pragma once
#include "SkBase.h"
#include "SkMemory.h"
#include "SkModel.h"
#include "SkTexture.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
class SkScene
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

    /** @brief Stores vertex layout components for model loading and Vulkan vertex input and atribute bindings  */
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
    SkModel model;
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
    VertexLayout layout;
    SkScene(/* args */){}
    ~SkScene(){}
    void Init(SkBase *initBase)
    {
        appBase = initBase;
        layout = {{VERTEX_COMPONENT_POSITION,
                   VERTEX_COMPONENT_NORMAL,
                   VERTEX_COMPONENT_UV}};
    }
    void ImportModel(const std::string &path, ModelCreateInfo *createInfo = nullptr)
    {
        model.Init(appBase);
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
                            model.verticesData.push_back(pPos->x * scale.x + center.x);
                            model.verticesData.push_back(-pPos->y * scale.y + center.y);
                            model.verticesData.push_back(pPos->z * scale.z + center.z);
                            break;
                        case VERTEX_COMPONENT_NORMAL:
                            model.verticesData.push_back(pNormal->x);
                            model.verticesData.push_back(-pNormal->y);
                            model.verticesData.push_back(pNormal->z);
                            break;
                        case VERTEX_COMPONENT_UV:
                            model.verticesData.push_back(pTexCoord->x * uvscale.s);
                            model.verticesData.push_back(pTexCoord->y * uvscale.t);
                            break;
                        case VERTEX_COMPONENT_COLOR:
                            model.verticesData.push_back(pColor.r);
                            model.verticesData.push_back(pColor.g);
                            model.verticesData.push_back(pColor.b);
                            break;
                        case VERTEX_COMPONENT_TANGENT:
                            model.verticesData.push_back(pTangent->x);
                            model.verticesData.push_back(pTangent->y);
                            model.verticesData.push_back(pTangent->z);
                            break;
                        case VERTEX_COMPONENT_BITANGENT:
                            model.verticesData.push_back(pBiTangent->x);
                            model.verticesData.push_back(pBiTangent->y);
                            model.verticesData.push_back(pBiTangent->z);
                            break;
                        // Dummy components for padding
                        case VERTEX_COMPONENT_DUMMY_FLOAT:
                            model.verticesData.push_back(0.0f);
                            break;
                        case VERTEX_COMPONENT_DUMMY_VEC4:
                            model.verticesData.push_back(0.0f);
                            model.verticesData.push_back(0.0f);
                            model.verticesData.push_back(0.0f);
                            model.verticesData.push_back(0.0f);
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

                uint32_t indexBase = static_cast<uint32_t>(model.indicesData.size());
                for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
                {
                    const aiFace &Face = paiMesh->mFaces[j];
                    if (Face.mNumIndices != 3)
                        continue;
                    model.indicesData.push_back(indexBase + Face.mIndices[0]);
                    model.indicesData.push_back(indexBase + Face.mIndices[1]);
                    model.indicesData.push_back(indexBase + Face.mIndices[2]);
                    parts[i].indexCount += 3;
                    indexCount += 3;
                }

                diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
                // textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
                // 2. specular maps
                specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
                // textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
                // 3. normal maps
                normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
                // textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
                // 4. height maps
                heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
                // textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
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
                texture.id->Init(appBase, str.C_Str());
                texture.type = typeName;
                texture.path = str.C_Str();
                _textures.push_back(texture);
                textures.push_back(texture); // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return _textures;
    }

    void Build(SkMemory *mem)
    {
        mem->BuildModel(&model);
        for (size_t i = 0; i < textures.size(); i++)
        {
            mem->BuildTexture(textures[i].id);
        }
    }
    VkDescriptorImageInfo GetTexDescriptor(int i)
    {

        VkDescriptorImageInfo texDescriptor = {};
        texDescriptor.sampler = textures[i].id->sampler;
        texDescriptor.imageLayout = textures[i].id->imageLayout;
        texDescriptor.imageView = textures[i].id->view;
        return texDescriptor;
    }
    void UsePipeline(SkGraphicsPipeline *pipeline)
    {
        pipeline->models.push_back(&model);
    }
    void CleanUp()
    {
        model.CleanUp();
        for (size_t i = 0; i < textures.size(); i++)
        {
            textures[i].id->CleanUp();
            delete textures[i].id;
            textures[i].id = nullptr;
        }
    }
};

