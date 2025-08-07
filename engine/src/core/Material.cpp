#include "core/Material.hpp"
#include <iostream>

Material::Material(MaterialType type) : type(type)
{
    // 确保所有材质类型都有合理的默认值
    if (type == PBR)
    {
        // PBR默认值
        albedo = glm::vec3(0.8f, 0.8f, 0.8f);
        metallic = 0.1f;
        roughness = 0.5f;
        ao = 1.0f;
    }
    else
    {
        // Blinn-Phong默认值
        diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
        specular = glm::vec3(0.5f, 0.5f, 0.5f);
        shininess = 32.0f;
    }
}

void Material::Bind(Shader &shader)
{
    shader.Use();

    if (type == BLINN_PHONG)
    {
        shader.SetVec3("material.diffuse", diffuse);
        shader.SetVec3("material.specular", specular);
        shader.SetFloat("material.shininess", shininess);

        if (useDiffuseMap)
        {
            diffuseMap->Bind(1);
            shader.SetBool("material.useDiffuseMap", 1);
            shader.SetInt("material.diffuseMap", 1);
        }
        else
        {
            shader.SetBool("material.useDiffuseMap", 0);
        }

        if (useSpecularMap)
        {
            specularMap->Bind(2);
            shader.SetBool("material.useSpecularMap", 1);
            shader.SetInt("material.specularMap", 2);
        }
        else
        {
            shader.SetBool("material.useSpecularMap", 0);
        }

        if (useNormalMap)
        {
            normalMap->Bind(3);
            shader.SetBool("material.useNormalMap", 1);
            shader.SetInt("material.normalMap", 3);
        }
        else
        {
            shader.SetBool("material.useNormalMap", 0);
        }

    }
    else
    { // PBR
        shader.SetVec3("material.albedo", albedo);
        shader.SetFloat("material.metallic", metallic);
        shader.SetFloat("material.roughness", roughness);
        shader.SetFloat("material.ao", ao);

        shader.SetBool("material.useAlbedoMap", useAlbedoMap);
        shader.SetBool("material.useMetallicMap", useMetallicMap);
        shader.SetBool("material.useRoughnessMap", useRoughnessMap);
        shader.SetBool("material.useAOMap", useAOMap);
        shader.SetBool("material.useNormalMap", useNormalMap);

        if (useAlbedoMap && albedoMap)
        {
            albedoMap->Bind(0);
            shader.SetInt("material.albedoMap", 0);
        }

        if (useMetallicMap && metallicMap)
        {
            metallicMap->Bind(1);
            shader.SetInt("material.metallicMap", 1);
        }

        if (useRoughnessMap && roughnessMap)
        {
            roughnessMap->Bind(2);
            shader.SetInt("material.roughnessMap", 2);
        }

        if (useAOMap && aoMap)
        {
            aoMap->Bind(3);
            shader.SetInt("material.aoMap", 3);
        }

        if (useNormalMap && normalMap)
        {
            normalMap->Bind(4);
            shader.SetInt("material.normalMap", 4);
        }
    }
}