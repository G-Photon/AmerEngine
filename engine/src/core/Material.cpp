#include "core/Material.hpp"

Material::Material(MaterialType type) : type(type)
{
}

void Material::Bind(Shader &shader)
{
    shader.Use();

    if (type == BLINN_PHONG)
    {
        shader.SetVec3("material.ambient", ambient);
        shader.SetVec3("material.diffuse", diffuse);
        shader.SetVec3("material.specular", specular);
        shader.SetFloat("material.shininess", shininess);

        if (useAmbientMap)
        {
            ambientMap->Bind(0);
            shader.SetBool("material.useAmbientMap", 1);
            shader.SetInt("material.ambientMap", 0);
        }
        else
        {
            shader.SetBool("material.useAmbientMap", 0);
        }

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

        if (albedoMap)
        {
            albedoMap->Bind(0);
            shader.SetInt("material.albedoMap", 0);
        }

        if (metallicMap)
        {
            metallicMap->Bind(1);
            shader.SetInt("material.metallicMap", 1);
        }

        if (roughnessMap)
        {
            roughnessMap->Bind(2);
            shader.SetInt("material.roughnessMap", 2);
        }

        if (aoMap)
        {
            aoMap->Bind(3);
            shader.SetInt("material.aoMap", 3);
        }

        if (normalMap)
        {
            normalMap->Bind(4);
            shader.SetInt("material.normalMap", 4);
        }
    }
}