#pragma once

#include "Shader.hpp"
#include <imgui.h>

class Light
{
  public:
    virtual ~Light() = default;
    virtual void OnInspectorGUI() = 0;
};

class PointLight : public Light
{
  public:
    PointLight(const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &color = glm::vec3(1.0f),
               float intensity = 1.0f);

    void SetupShader(Shader &shader, int index) const;
    void OnInspectorGUI() override;

    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

class DirectionalLight : public Light
{
  public:
    DirectionalLight(const glm::vec3 &direction = glm::vec3(-0.2f, -1.0f, -0.3f),
                     const glm::vec3 &color = glm::vec3(1.0f), float intensity = 1.0f);

    void SetupShader(Shader &shader) const;
    void OnInspectorGUI() override;

    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
};

class SpotLight : public Light
{
  public:
    SpotLight(const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &direction = glm::vec3(1.0f, 0.0f, 0.0f),
              const glm::vec3 &color = glm::vec3(1.0f), float intensity = 1.0f, float cutOff = 12.5f,
              float outerCutOff = 17.5f);

    void SetupShader(Shader &shader, int index) const;
    void OnInspectorGUI() override;

    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};