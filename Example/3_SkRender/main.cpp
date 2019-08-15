#include "SkApp.h"
#include "stdexcept"
#include "iostream"
#include "SkRayTracing.h"
#include "SkSVGF.h"
#include "SkMaterial.h"
#ifdef NDEBUG

const bool validation = false;
#else
const bool validation = true;
#endif
// const bool validation = true;

class SkRender : public SkApp
{
    struct
    {
        VkDeviceMemory memory; // Handle to the device memory for this buffer
        VkBuffer buffer;       // Handle to the Vulkan buffer object that the memory is bound to
    } vertices;

    // Index buffer
    struct
    {
        VkDeviceMemory memory;
        VkBuffer buffer;
        uint32_t count;
    } indices;

    SkModel model;
    SkLightSet lights;
    SkTexture *pTexture;
    SkGraphicsPipeline gBufferPipeline;
    SkGraphicsPipeline denoisePipeline;
    SkRayTracing ray;
    SkSVGF svgf;
    VkSampler sampler;
    void PrepareScene()
    {
        model.Init(appBase,&mem);
        model.ImportModel("vk.obj");
        lights.Init(&mem);
        lights.AddPointLight(glm::vec3(0.0f));
        lights.lights[0].radius=1.0f;
        lights.Setup();
        for (size_t i = 0; i < model.meshes.size(); i++)
        {
             fprintf(stderr, "mesh[%zd]: %zd,%zd,%d...\n",i, model.meshes[i].verticesData.size(), model.meshes[i].indicesData.size(), model.meshes[i].stride);
        }
    }
    void PreparePipeline()
    {
        gBufferPipeline.Init(appBase);
        std::vector<VkDescriptorPoolSize> poolSizes = {
            SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10),
            SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10),
            SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10),
        };
        VkDescriptorPool pool = gBufferPipeline.CreateDescriptorPool(poolSizes, 10);

        std::vector<VkDescriptorSetLayoutBinding> bindings;

        bindings.clear();
        bindings = {
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
            };
        SkMaterial::AddMatBinding(bindings);
        gBufferPipeline.SetShader("Shader\\vert_3_gbuffer.spv", "Shader\\frag_3_gbuffer.spv");
        gBufferPipeline.CreateDescriptorSetLayout(bindings);
        gBufferPipeline.CreateGraphicsPipeline(0, 4, &model.inputBindings, &model.inputAttributes);

        denoisePipeline.Init(appBase, true, pool);
        bindings.clear();
        bindings = {
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 6),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 7),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 8),
            SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 9),
        };

        denoisePipeline.SetShader("Shader/vert_3_denoise.spv", "Shader/frag_3_denoise.spv");
        denoisePipeline.CreateDescriptorSetLayout(bindings);
        denoisePipeline.CreateGraphicsPipeline(1, 1);
    }
    void PrepareCmd()
    {
        model.Build();
        PrepareRayTracing();
        model.UsePipeline(&gBufferPipeline);
        cmd.RegisterPipeline(&gBufferPipeline, 0);
        cmd.RegisterPipeline(&denoisePipeline, 1);
        mem.CreateSampler(&sampler);
        RewriteDescriptorSet(true);
        this->cmd.CreateCmdBuffers();
    }
    void PrepareRayTracing()
    {
        ray.Init(appBase, &mem);
        ray.CreateScene(model.meshes,&lights);
        ray.CreateStorageImage();
        ray.CreateUniformBuffer();
        ray.CreateRayTracingPipeline();
        ray.CreateShaderBindingTable();
        ray.CreateDescriptorSets();
        ray.BuildCommandBuffers();

        svgf.Init(appBase, &mem);
        svgf.Register(&appBase->position);
        svgf.Register(&appBase->normal);
        svgf.Register(&appBase->albedo);
        svgf.Build();
    }
    void UpdateLight()
    {
        lights.lights[0].pos=glm::vec4(cos(glm::radians(appBase->currentTime * 36.0)) * 40.0f, -40.0f + sin(glm::radians(appBase->currentTime * 36.0)) * 20.0f, 15.0f + sin(glm::radians(appBase->currentTime * 36.0)) * 5.0f, 0.0f);
        lights.Update();
    }
    void BeforeDraw(uint32_t imageIndex) override
    {
        callback.UpdataBuffer();
        UpdateLight();
        ray.UpdateUniformBuffers();
        ray.Submit(imageIndex);
    }
    void AfterDraw() override
    {
        svgf.Submit(appBase->currentFrame);
    }
    void RewriteDescriptorSet(bool alloc = false) override
    {
        std::vector<VkWriteDescriptorSet> writeSets = {
            SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &appBase->vpBuffer.descriptor), 
            };
        model.SetupDescriptorSet(&gBufferPipeline,writeSets,alloc);
        gBufferPipeline.PrepareDynamicState();
        fprintf(stderr,"write gbuffer des...OK!\n");

        VkDescriptorImageInfo positionDes = {};
        positionDes.sampler = sampler;
        positionDes.imageView = appBase->position.view;
        positionDes.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo normalDes = {};
        normalDes.sampler = sampler;
        normalDes.imageView = appBase->normal.view;
        normalDes.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo albedoDes = {};
        albedoDes.sampler = sampler;
        albedoDes.imageView = appBase->albedo.view;
        albedoDes.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkDescriptorImageInfo rtImage = ray.GetReadDescriptor();
        writeSets.clear();
        writeSets =
            {
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &positionDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &normalDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &albedoDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &rtImage),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, svgf.GetDes()),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, svgf.GetDes(0)),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, svgf.GetDes(1)),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, svgf.GetDes(2)),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8, svgf.GetVPDes()),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 9, &appBase->inverseBuffer.descriptor),
            };
        denoisePipeline.SetupDescriptorSet(nullptr,writeSets, alloc);
        denoisePipeline.PrepareDynamicState();
        fprintf(stderr,"write denoise des...OK!\n");

    }

public:
    SkRender() : SkApp("SkRender", validation)
    {
        appBase->enableInstanceExtensions.insert(
            appBase->enableInstanceExtensions.end(),
            RTInstanceExtensions.begin(), RTInstanceExtensions.end());

        appBase->enableDeviceExtensions.insert(
            appBase->enableDeviceExtensions.end(),
            RTDeviceExtensions.begin(), RTDeviceExtensions.end());
    }
    void AppSetup() override
    {
        SkApp::AppSetup();
        PrepareScene();
        PreparePipeline();
        PrepareCmd();
    }
    void CleanUp1() override
    {
        SkApp::CleanUp1();
        svgf.CleanUp();
        lights.CleanUp();
        vkDestroySampler(appBase->device, sampler, nullptr);
        gBufferPipeline.CleanUp();
        denoisePipeline.CleanUp();
        model.CleanUp();
        ray.CleanUp();
    }
};

int main()
{
    SkRender skApp;
    try
    {
        skApp.Run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}