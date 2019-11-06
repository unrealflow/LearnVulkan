#pragma once
#include "SkBase.h"
#include "SkAgent.h"
#include "SkTexture.h"
#include "assimp/material.h"
#include "assimp/ai_assert.h"
//仿照Blender中的材质定义
//暂时仅使用了部分定义
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
    float emission;
    float IOR;
    float transmission;

    float useTex;
    glm::vec3 _PAD_;
};
class SkMaterial
{
private:
    SkAgent *agent;
    struct Texture
    {
        SkTexture *id;
        std::string type;
        std::string path;
    };
    //传入Shader的各种数据的绑定位置

    //为贴图分配显存空间
    void BuildTexture(SkTexture *tex, bool useStaging = false);

public:
    //所有贴图的集合
    std::vector<Texture> textures;
    //漫反射贴图集合
    std::vector<Texture> diffuseMaps;
    SkMat mat;
    //储存属性
    SkBuffer matBuf;
    //储存贴图
    SkImage matTex;
    void Init(SkAgent *initAgent);
    //根据Assimp导入的模型信息来生成材质
    void LoadMaterial(aiMaterial *mat, std::string dir = ".");
    void LoadDefaultTex();

    std::vector<Texture> LoadMaterialTextures(aiMaterial *mat,
                                              std::string dir,
                                              aiTextureType type,
                                              std::string typeName);
    //将数据写入显存
    void Build();
    //将材质的管线绑定信息添加至bindings中
    static void AddMatBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings);
    //将用于光线追踪的绑定信息添加至bindings中
    static void AddRayMatBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings, uint32_t index = 0);
    //将材质需要传入shader的信息添加至writeSets
    //desSet为接收写入信息的DescriptorSet
    //若使用writeSets的对象为SkMesh，则desSet会被替换为mesh的descriptorSet，此处的desSet将不起作用
    void SetWriteDes(std::vector<VkWriteDescriptorSet> &writeSets, VkDescriptorSet desSet = 0, uint32_t index = 0);

    void CleanUp();

    SkMaterial(){};
    ~SkMaterial(){};
};
class SkMatSet
{
    std::vector<SkMaterial> matSet;
    SkAgent *mem;

public:
    void Init(SkAgent *initAgent);
    //添加至Set后，原mat应不再使用
    uint32_t AddMat(SkMaterial &mat)
    {
        mat.LoadDefaultTex();
        matSet.emplace_back(mat);
        return static_cast<uint32_t>(matSet.size()) - 1;
    }
    uint32_t GetCount()
    {
        return static_cast<uint32_t>(matSet.size());
    }
    //根据Assimp导入的模型信息来生成材质
    uint32_t AddMat(aiMaterial *mat, std::string dir = ".")
    {
        SkMaterial tmp;
        tmp.Init(mem);
        tmp.LoadMaterial(mat, dir);
        matSet.emplace_back(tmp);
        return static_cast<uint32_t>(matSet.size()) - 1;
    }
    //将数据写入显存
    void Build()
    {
        for (size_t i = 0; i < matSet.size(); i++)
        {
            matSet[i].Build();
        }
    }
    SkMaterial *GetMat(uint32_t index)
    {
        if (index >= matSet.size())
        {
            return nullptr;
        }
        return &matSet[index];
    }
    void CleanUp()
    {
        for (size_t i = 0; i < matSet.size(); i++)
        {
            matSet[i].CleanUp();
        }
    }
};
