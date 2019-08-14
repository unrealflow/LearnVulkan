#include "SkMaterial.h"

void SkMaterial::Init(SkMemory *initMem)
{
    mem = initMem;
    mat.baseColor = glm::vec3(1.0f);
    mat.metallic = 0.0f;
    mat.subsurface = 0.0f;
    mat.specular = 0.5f;
    mat.roughness = 0.1f;
    mat.specularTint = 0.0f;
    mat.anisotropic = 0.0f;
    mat.sheen = 0.0f;
    mat.sheenTint = 0.5f;
    mat.clearcoat = 0.0f;
    mat.clearcoatGloss = 0.0f;
    mat.emission=0.3f;
}
void SkMaterial::BuildTexture(SkTexture *tex, bool useStaging )
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
std::vector<SkMaterial::Texture> SkMaterial::LoadMaterialTextures(aiMaterial *mat,
                                                                  std::string dir,
                                                                  aiTextureType type,
                                                                  std::string typeName)
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
            texture.id->Init(dir + '/' + str.C_Str());
            texture.type = typeName;
            texture.path = str.C_Str();
            _textures.push_back(texture);
            textures.push_back(texture); // store it as texture loaded for entire mesh, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return _textures;
}
void SkMaterial::AddMatBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings)
{
    bindings.emplace_back(
        SkInit::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_FRAGMENT_BIT, LOC::UNIFORM));
    bindings.emplace_back(
        SkInit::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT, LOC::DIFFUSE));
}
void SkMaterial::AddRayMatBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings,uint32_t index)
{
    bindings.emplace_back(
        SkInit::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, LOC::UNIFORM+index*LOC::STRIDE));
    bindings.emplace_back(
        SkInit::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, LOC::DIFFUSE+index*LOC::STRIDE));
}
void SkMaterial::SetWriteDes(std::vector<VkWriteDescriptorSet> &writeSets, VkDescriptorSet desSet,uint32_t index)
{

    writeSets.emplace_back(
        SkInit::writeDescriptorSet(
            desSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            LOC::UNIFORM+index*LOC::STRIDE,
            &matBuf.descriptor));
    if (diffuseMaps.size() > 0)
    {
        writeSets.emplace_back(
            SkInit::writeDescriptorSet(
                desSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                LOC::DIFFUSE+index*LOC::STRIDE,
                &(diffuseMaps[0].id->image.descriptor)));
    }
}