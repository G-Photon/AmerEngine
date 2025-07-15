#pragma once

#include "Texture.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <core/Shader.hpp>

enum MaterialType
{
    BLINN_PHONG,
    PBR
};

class Material
{
  public:
    Material(MaterialType type = PBR);

    void Bind(Shader &shader);

    // Blinn-Phong 参数
    glm::vec3 ambient = glm::vec3(0.1f);
    glm::vec3 diffuse = glm::vec3(0.8f);
    glm::vec3 specular = glm::vec3(0.5f);
    float shininess = 32.0f;

    // PBR 参数
    glm::vec3 albedo = glm::vec3(0.8f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;

    // 贴图
    std::shared_ptr<Texture> ambientMap;
    std::shared_ptr<Texture> diffuseMap;
    std::shared_ptr<Texture> specularMap;
    std::shared_ptr<Texture> normalMap;

    std::shared_ptr<Texture> albedoMap;
    std::shared_ptr<Texture> metallicMap;
    std::shared_ptr<Texture> roughnessMap;
    std::shared_ptr<Texture> aoMap;

    MaterialType type;
    std::string name = "New Material";
};