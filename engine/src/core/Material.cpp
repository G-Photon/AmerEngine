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

        if (ambientMap)
        {
            ambientMap->Bind(0);
            shader.SetInt("material.ambientMap", 0);
        }

        if (diffuseMap)
        {
            diffuseMap->Bind(1);
            shader.SetInt("material.diffuseMap", 1);
        }

        if (specularMap)
        {
            specularMap->Bind(2);
            shader.SetInt("material.specularMap", 2);
        }

        if (normalMap)
        {
            normalMap->Bind(3);
            shader.SetInt("material.normalMap", 3);
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