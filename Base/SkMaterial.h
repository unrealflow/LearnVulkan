#pragma once
#include "SkBase.h"
#include "SkMemory.h"
#include "SkTexture.h"
#include "assimp/material.h"
#include "assimp/ai_assert.h"

class SkMaterial
{
private:
    SkMemory *mem;
    struct Texture
    {
        SkTexture *id;
        std::string type;
        std::string path;
    };
    enum BINDING
    {
        UNIFORM = 100,
        DIFFUSE = 101
    };
    void BuildTexture(SkTexture *tex, bool useStaging = false);

public:
    std::vector<Texture> textures;
    std::vector<Texture> diffuseMaps;
    struct SkMat
    {
        glm::vec3 baseColor;
        float metallic;
        float subsurface;
        float specular;
        float roughness;
        float specularTint;
        float anisotropic;
        float sheen;
        float sheenTint;
        float clearcoat;
        float clearcoatGloss;
        float indirect;
    } mat;
    SkBuffer matBuf;
    SkImage matTex;
    void Init(SkMemory *initMem);

    void LoadMaterial(aiMaterial *mat, std::string dir = ".")
    {
        diffuseMaps = LoadMaterialTextures(mat, dir, aiTextureType_DIFFUSE, "texture_diffuse");
    }
    std::vector<Texture> LoadMaterialTextures(aiMaterial *mat,
                                              std::string dir,
                                              aiTextureType type,
                                              std::string typeName);
    void Build()
    {
        mem->CreateLocalBuffer(&mat, sizeof(mat), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &matBuf);
        mem->SetupDescriptor(&matBuf);
        for (size_t i = 0; i < textures.size(); i++)
        {
            BuildTexture(textures[i].id, false);
        }
    }

    static void AddMatBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings);

    static void AddRayMatBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings);

    void SetWriteDes(std::vector<VkWriteDescriptorSet> &writeSets, VkDescriptorSet desSet = 0);

    void CleanUp()
    {
        for (size_t i = 0; i < textures.size(); i++)
        {
            mem->FreeImage(&(textures[i].id->image));
            delete textures[i].id;
            textures[i].id = nullptr;
        }
        mem->FreeBuffer(&matBuf);
    }
    SkMaterial(){};
    ~SkMaterial(){};
};
