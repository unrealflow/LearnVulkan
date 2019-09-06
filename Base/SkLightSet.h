#pragma once
#include "SkMemory.h"

#define SK_LIGHT_TYPE_POINT 0.0f,
#define SK_LIGHT_TYPE_DIRCT 1.0f,
#define SK_LIGHT_TYPE_AMBIE 2.0f,

struct SkLight : public BLight
{
    SkLight(BLight &blight)
    {
        type = blight.type;
        pos = blight.pos;
        dir = blight.dir;
        color = blight.color;
        radius = blight.radius;
        atten = blight.atten;
    }
    SkLight(glm::vec3 _pos) : SkLight(0.0, _pos,
                                      glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::vec3(1.0f, 1.0f, 1.0f),
                                      0.0f, 0.0f)
    {
    }
    SkLight(float _type, glm::vec3 _pos,
            glm::vec3 _dir, glm::vec3 _color,
            float _radius, float _atten)
    {
        type = _type;
        pos = _pos;
        dir = _dir;
        color = _color;
        radius = _radius;
        atten = _atten;
    }
    static SkLight CreatePointLight(glm::vec3 pos, glm::vec3 color, float radius = 0, float atten = 0)
    {
        return SkLight(0.0, pos, glm::vec3(0.0f, -1.0f, 0.0f), color, radius, atten);
    }
    static SkLight CreateDirctionLight(glm::vec3 dir, glm::vec3 color)
    {
        return SkLight(1.0, glm::vec3(0.0f), dir, color, 0.0f, 0.0f);
    }
    static SkLight CreateAmbientLight(glm::vec3 color)
    {
        return SkLight(2.0, glm::vec3(0.0f), glm::vec3(0.0f), color, 0.0f, 0.0f);
    }
};

class SkLightSet
{
private:
    SkMemory *mem;
    SkBuffer buffer;

public:
    std::vector<SkLight> lights;
    void Init(SkMemory *initMem)
    {
        mem = initMem;
        lights.clear();
    }
    uint32_t AddLight(SkLight light)
    {
        lights.emplace_back(light);
        return static_cast<uint32_t>(lights.size()) - 1;
    }
    uint32_t AddPointLight(glm::vec3 pos)
    {
        lights.emplace_back(SkLight(pos));
        return static_cast<uint32_t>(lights.size()) - 1;
    }
    void ImportLights(BScene *scene)
    {
        if (nullptr == scene)
        {
            throw std::exception("No Scene Data");
        }
        if (0 == scene->lightCount)
        {
            fprintf(stderr, "No Light! Add Default Light...\n");
            lights.emplace_back(SkLight::CreateAmbientLight(glm::vec3(0.5f)));
        }
        for (uint32_t i = 0; i < scene->lightCount; i++)
        {
            lights.emplace_back(SkLight(scene->lights[i]));
        }
    }
    void Setup()
    {
        mem->CreateBuffer(lights.data(), lights.size() * sizeof(SkLight), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &buffer);
        mem->SetupDescriptor(&buffer);
        mem->Map(&buffer);
    }
    void AddLightBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings)
    {
        bindings.emplace_back(
            SkInit::descriptorSetLayoutBinding(
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, LOC::LIGHT));
    }
    void SetWriteDes(std::vector<VkWriteDescriptorSet> &writeSets, VkDescriptorSet desSet)
    {
        writeSets.emplace_back(
            SkInit::writeDescriptorSet(
                desSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                LOC::LIGHT,
                &buffer.descriptor));
    }
    void Update()
    {
        if (buffer.data == nullptr)
        {
            throw std::exception("ERROR : Lights Buffer not Setup!");
        }
        memcpy(buffer.data, lights.data(), lights.size() * sizeof(SkLight));
    }
    void CleanUp()
    {
        mem->FreeBuffer(&buffer);
    }
    SkLightSet() {}
    ~SkLightSet() {}
};
