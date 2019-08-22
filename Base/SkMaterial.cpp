#include "SkMaterial.h"

static SkTexture *defaultTex = nullptr;
static bool builded = false;

void SkMaterial::Init(SkMemory *initMem)
{
    mem = initMem;
    mat.baseColor = glm::vec3(1.0f);
    mat.metallic = 0.0f;
    mat.subsurface = 0.0f;
    mat.specular = 0.0f;
    mat.roughness = 0.5f;
    mat.specularTint = 0.0f;
    mat.anisotropic = 0.0f;
    mat.sheen = 0.0f;
    mat.sheenTint = 0.5f;
    mat.clearcoat = 0.0f;
    mat.clearcoatGloss = 0.0f;
    mat.emission = 0.1f;
    mat.useTex = -1.0;
}
void SkMaterial::BuildTexture(SkTexture *tex, bool useStaging)
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
void SkMaterial::Build()
{
    mem->CreateLocalBuffer(&mat, sizeof(mat), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &matBuf);
    mem->SetupDescriptor(&matBuf);
    for (size_t i = 0; i < textures.size(); i++)
    {
        BuildTexture(textures[i].id, false);
    }
    if (defaultTex != nullptr && builded == false)
    {
        BuildTexture(defaultTex, false);
        builded = true;
    }
}
void SkMaterial::LoadMaterial(aiMaterial *mat, std::string dir)
{
    aiColor3D pColor(0.f, 0.f, 0.f);
    mat->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);
    this->mat.baseColor.x = pColor.r;
    this->mat.baseColor.y = pColor.g;
    this->mat.baseColor.z = pColor.b;
    diffuseMaps = LoadMaterialTextures(mat, dir, aiTextureType_DIFFUSE, "texture_diffuse");
    if (diffuseMaps.size() > 0)
    {
        this->mat.useTex = 1.0;
    }
    else
    {
        this->mat.useTex = -1.0;
        Texture defTex = {};
        if (defaultTex == nullptr)
        {
            defaultTex = new SkTexture();
            defaultTex->Init("my.jpg");
        }
        // directory + '/' +
        defTex.id = defaultTex;
        defTex.type = "texture_diffuse";
        defTex.path = "my.jpg";
        diffuseMaps.push_back(defTex);
    }
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
void SkMaterial::AddRayMatBinding(std::vector<VkDescriptorSetLayoutBinding> &bindings, uint32_t index)
{
    bindings.emplace_back(
        SkInit::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, LOC::UNIFORM + index * LOC::STRIDE));
    bindings.emplace_back(
        SkInit::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV, LOC::DIFFUSE + index * LOC::STRIDE));
}
void SkMaterial::SetWriteDes(std::vector<VkWriteDescriptorSet> &writeSets, VkDescriptorSet desSet, uint32_t index)
{

    writeSets.emplace_back(
        SkInit::writeDescriptorSet(
            desSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            LOC::UNIFORM + index * LOC::STRIDE,
            &matBuf.descriptor));
    if (diffuseMaps.size() > 0)
    {
        writeSets.emplace_back(
            SkInit::writeDescriptorSet(
                desSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                LOC::DIFFUSE + index * LOC::STRIDE,
                &(diffuseMaps[0].id->image.descriptor)));
    }
}
void SkMaterial::CleanUp()
{
    for (size_t i = 0; i < textures.size(); i++)
    {
        mem->FreeImage(&(textures[i].id->image));
        delete textures[i].id;
        textures[i].id = nullptr;
    }
    if (defaultTex != nullptr)
    {
        mem->FreeImage(&defaultTex->image);
        delete defaultTex;
        defaultTex = nullptr;
    }
    mem->FreeBuffer(&matBuf);
}