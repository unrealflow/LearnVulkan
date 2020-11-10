#include "SkApp.h"
#include "SkRayTracing.h"

class SkRender : public SkApp
{
private:
    SkModel model;
    SkLightSet lights;
    SkTexture *pTexture;
    SkSVGF svgf;
    SkGraphicsPipeline gBufferPipeline;
    std::array<SkGraphicsPipeline, PASS_COUNT> pipelines;
    SkRayTracing ray;
    void PrepareScene()
    {
        model.Init(&agent);
        lights.Init(&agent);
        auto info = SkModel::ModelCreateInfo();
        info.scale = glm::vec3(2.0f);
        bool use_big_model=false;
        // use_big_model=true;
        if(use_big_model)
        {
            model.ImportModel("Model/model.obj", &info);
            model.matSet.GetMat(1)->mat.roughness = 0.1f;
            model.matSet.GetMat(1)->mat.metallic = 0.3f;
            model.matSet.GetMat(3)->mat.roughness = 0.1f;
            model.matSet.GetMat(3)->mat.metallic = 0.8f;
            // model.matSet.GetMat(1)->mat.transmission = 0.9f;
            lights.AddPointLight(
                glm::vec3(
                    cos(glm::radians(60.0f)) * 10.0f,
                    -12.0f + sin(glm::radians(60.0f)) * 5.0f,
                    6.0f + sin(glm::radians(60.0f)) * 2.0f));
            lights.lights[0].type = 0.0f;
            lights.lights[0].dir = glm::vec3(-0.5f, 0.7f, -0.4f);
            lights.lights[0].radius = 10.0f;
            lights.lights[0].color = glm::vec3(22000.0f);
            lights.lights[0].atten = 2.0f;
        } else
        {
            model.ImportModel("Model/glass.obj");
            model.matSet.GetMat(3)->mat.transmission = 0.9f;
            lights.AddPointLight(
                glm::vec3(
                    4.0f,
                    -4.0f,
                    0.0f));
            lights.lights[0].type = 0.0f;
            lights.lights[0].dir = glm::vec3(-0.5f, 0.7f, -0.4f);
            lights.lights[0].radius = 1.0f;
            lights.lights[0].color = glm::vec3(2800.0f);
            lights.lights[0].atten = 2.0f;
        }
        lights.Setup();
        for (size_t i = 0; i < model.meshes.size(); i++)
        {
            fprintf(stderr, "mesh[%zd]: %zd,%zd,%d...\n", i, model.meshes[i].verticesData.size(), model.meshes[i].indicesData.size(), model.meshes[i].stride);
        }
    }

    void PreparePipeline()
    {
        VkDescriptorPool pool;
        {
            gBufferPipeline.Init(appBase);
            std::vector<VkDescriptorPoolSize> poolSizes = {
                SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10),
                SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20),
                SkInit::descriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10),
            };
            pool = gBufferPipeline.CreateDescriptorPool(poolSizes, 60);

            std::vector<VkDescriptorSetLayoutBinding> bindings = {
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
                // SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
            };
            SkMaterial::AddMatBinding(bindings);
            gBufferPipeline.SetShader("Shader\\gbuffer.vert.spv", "Shader\\gbuffer.frag.spv");
            gBufferPipeline.CreateDescriptorSetLayout(bindings);
            gBufferPipeline.CreateGraphicsPipeline(0, 3, &model.inputBindings, &model.inputAttributes);
        }
        {
            pipelines[0].Init(appBase, true, pool);
            std::vector<VkDescriptorSetLayoutBinding> bindings = {
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),//rtImage

            };
            model.AddAllBinding(bindings);
            lights.AddLightBinding(bindings);
            pipelines[0].SetShader("Shader/pass.vert.spv", "Shader/pass0.frag.spv");
            pipelines[0].CreateDescriptorSetLayout(bindings);
            pipelines[0].CreateGraphicsPipeline(1, 1);
        }
        {
            pipelines[1].Init(appBase, true, pool);
            std::vector<VkDescriptorSetLayoutBinding> bindings = {
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
            };

            pipelines[1].SetShader("Shader/pass.vert.spv", "Shader/pass1.frag.spv");
            pipelines[1].CreateDescriptorSetLayout(bindings);
            pipelines[1].CreateGraphicsPipeline(2, 1);
        }
        {
            pipelines[2].Init(appBase, true, pool);
            std::vector<VkDescriptorSetLayoutBinding> bindings = {
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5),
                

            };

            pipelines[2].SetShader("Shader/pass.vert.spv", "Shader/pass2.frag.spv");
            pipelines[2].CreateDescriptorSetLayout(bindings);
            pipelines[2].CreateGraphicsPipeline(3, 1);
        }
        {
            pipelines[3].Init(appBase, true, pool);
            std::vector<VkDescriptorSetLayoutBinding> bindings = {
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
            };

            pipelines[3].SetShader("Shader/pass.vert.spv", "Shader/pass3.frag.spv");
            pipelines[3].CreateDescriptorSetLayout(bindings);
            pipelines[3].CreateGraphicsPipeline(4, 1);
        }
        {
            pipelines[4].Init(appBase, true, pool);
            std::vector<VkDescriptorSetLayoutBinding> bindings = {
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
            };

            pipelines[4].SetShader("Shader/pass.vert.spv", "Shader/pass4.frag.spv");
            pipelines[4].CreateDescriptorSetLayout(bindings);
            pipelines[4].CreateGraphicsPipeline(5, 1);
        }    
        {
            pipelines[5].Init(appBase, true, pool);
            std::vector<VkDescriptorSetLayoutBinding> bindings = {
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
                SkInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
            };

            pipelines[5].SetShader("Shader/pass.vert.spv", "Shader/pass5.frag.spv");
            pipelines[5].CreateDescriptorSetLayout(bindings);
            pipelines[5].CreateGraphicsPipeline(6, 1);
        }     
    }
    void RewriteDescriptorSet(bool alloc = false) override
    {
        VkDescriptorImageInfo positionDes = agent.SetupImageInfo(&appBase->position);
        VkDescriptorImageInfo normalDes = agent.SetupImageInfo(&appBase->normal);
        VkDescriptorImageInfo albedoDes = agent.SetupImageInfo(&appBase->albedo);
        VkDescriptorImageInfo passDes[PASS_COUNT];
        for (int i = 0; i < PASS_COUNT - 1; i++)
        {
            passDes[i] = agent.SetupImageInfo(&appBase->pass[i]);
        }

        std::vector<VkWriteDescriptorSet> writeSets = {
            SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &appBase->UBO.descriptor),
        };
        model.SetupDescriptorSet(&gBufferPipeline, writeSets, alloc);
        gBufferPipeline.PrepareDynamicState();
        fprintf(stderr, "write gbuffer des...OK!\n");
        VkDescriptorImageInfo rtImage=ray.GetReadDescriptor();
        writeSets.clear();
        writeSets =
            {
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &positionDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &normalDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &albedoDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &appBase->UBO.descriptor),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &rtImage),
            };
        model.SetAllWirteDes(writeSets, 0);
        lights.SetWriteDes(writeSets, 0);
        pipelines[0].SetupDescriptorSet(nullptr, writeSets, alloc);
        pipelines[0].PrepareDynamicState();
        fprintf(stderr, "write pass0 des...OK!\n");

        writeSets.clear();
        writeSets =
            {
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &positionDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &normalDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &albedoDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &passDes[0]),
            };
        pipelines[1].SetupDescriptorSet(nullptr, writeSets, alloc);
        pipelines[1].PrepareDynamicState();
        fprintf(stderr, "write pass1 des...OK!\n");

        // writeSets.emplace_back(
        //     SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 4, &pass1Des));
        writeSets.clear();
        
        writeSets =
            {
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0, &positionDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, &normalDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &albedoDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &passDes[1]),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &appBase->UBO.descriptor),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, svgf.GetDes(0)),

            };
        pipelines[2].SetupDescriptorSet(nullptr, writeSets, alloc);
        pipelines[2].PrepareDynamicState();
        fprintf(stderr, "write pass2 des...OK!\n");

        writeSets.clear();
        writeSets =
            {
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &positionDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &normalDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &albedoDes),
                SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &passDes[2]),
            };
        pipelines[3].SetupDescriptorSet(nullptr, writeSets, alloc);
        pipelines[3].PrepareDynamicState();
        fprintf(stderr, "write pass3 des...OK!\n");

        writeSets[3]=SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &passDes[3]);
        pipelines[4].SetupDescriptorSet(nullptr, writeSets, alloc);
        pipelines[4].PrepareDynamicState();
        fprintf(stderr, "write pass4 des...OK!\n");

        writeSets[3]=SkInit::writeDescriptorSet(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &passDes[4]);
        pipelines[5].SetupDescriptorSet(nullptr, writeSets, alloc);
        pipelines[5].PrepareDynamicState();
        fprintf(stderr, "write pass5 des...OK!\n");
    }
    void PrepareRayTracing()
    {
        ray.Init(appBase, &agent);
        ray.CreateScene(model.meshes, &lights);
        ray.CreateStorageImage();
        ray.CreateUniformBuffer();
        ray.CreateRayTracingPipeline();
        ray.CreateShaderBindingTable();
        ray.CreateDescriptorSets();
        ray.BuildCommandBuffers();
    }
    void PrepareCmd()
    {
        svgf.Init(appBase, &agent);
        // svgf.Register(&appBase->position);
        // svgf.Register(&appBase->normal);
        // svgf.Register(&appBase->albedo);
        svgf.Register(&appBase->pass[2]);
        svgf.Build();

        model.Build();
        model.UsePipeline(&gBufferPipeline);
        PrepareRayTracing();
        cmd.RegisterPipeline(&gBufferPipeline, 0);
        for (int i = 0; i < PASS_COUNT; i++)
        {
            if (pipelines[i].pipeline == VK_NULL_HANDLE)
            {
                throw std::runtime_error("Need Create Pipeline");
            }
            cmd.RegisterPipeline(&pipelines[i], i + 1);
        }
        RewriteDescriptorSet(true);
        cmd.CreateCmdBuffers();
    }
    void Resize0() override
    {
        svgf.CleanUp();
        svgf.Init(appBase, &agent);
        // svgf.Register(&appBase->position);
        // svgf.Register(&appBase->normal);
        // svgf.Register(&appBase->albedo);
        svgf.Register(&appBase->pass[2]);
        svgf.Build();
    }

    void BeforeDraw(uint32_t imageIndex) override
    {
        callback.UpdataBuffer();
        lights.Update();
        ray.UpdateUniformBuffers();
        ray.Submit(imageIndex);
    }
    void AfterDraw() override
    {
        svgf.Submit(appBase->currentFrame);
    }

public:
    SkRender() : SkApp("SkRender",true)
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
        PrepareScene();
        PreparePipeline();
        PrepareCmd();
    }
    void CleanUp0() override
    {
        svgf.CleanUp();
        lights.CleanUp();
        model.CleanUp();
        ray.CleanUp();
        gBufferPipeline.CleanUp();
        for (int i = 0; i < PASS_COUNT; i++)
        {
            pipelines[i].CleanUp();
        }
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