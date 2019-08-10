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
    //各buffer和texture的绑定位置
    enum BINDING
    {
        //SkMat属性
        UNIFORM = 100,
        //漫反射贴图
        DIFFUSE = 101
    };
    //为贴图分配显存空间
    void BuildTexture(SkTexture *tex, bool useStaging = false);

public:
    //所有贴图的集合
    std::vector<Texture> textures;
    //漫反射贴图集合
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
    //储存属性
    SkBuffer matBuf;
    //储存贴图
    SkImage matTex;
    void Init(SkMemory *initMem);
    //根据Assimp导入的模型信息来生成材质
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
    //将材质的管线绑定信息添加至bindings中
    static void AddMatBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings);
    //将用于光线追踪的绑定信息添加至bindings中
    static void AddRayMatBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings);
    //将材质需要传入shader的信息添加至writeSets
    //desSet为接收写入信息的DescriptorSet
    //若使用writeSets的对象为SkMesh，则desSet会被替换为mesh的descriptorSet，此处的desSet将不起作用
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
