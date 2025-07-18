#pragma once

#include "Shader.hpp"
#include <imgui.h>

class Light
{
  public:
    virtual ~Light() = default;
    virtual void OnInspectorGUI() = 0;
    virtual void SetupShader(Shader &shader, int index = 0) const = 0;
    virtual int getNum() const
    {
        return 1; // 默认返回1，子类可以重载此方法
    }

    virtual void RemoveFromScene(int index = 0)
    {
        // 默认实现为空，子类可以重载此方法
    }

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 diffuse = glm::vec3(1.0f);
};

class PointLight : public Light
{
  public:
    PointLight(const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &ambient = glm::vec3(1.0f), const glm::vec3 &diffuse = glm::vec3(1.0f), const glm::vec3 &specular = glm::vec3(1.0f), float intensity = 1.0f);

    void SetupShader(Shader &shader, int index) const override;
    void OnInspectorGUI() override;
    int getNum() const override
    {
        return number;
    }

    int number;
    static int count;

    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float intensity;
    float constant;
    float linear;
    float quadratic;
};

class DirectionalLight : public Light
{
  public:
    DirectionalLight(const glm::vec3 &direction = glm::vec3(-0.2f, -1.0f, -0.3f),
                     const glm::vec3 &ambient = glm::vec3(1.0f), const glm::vec3 &diffuse = glm::vec3(1.0f), const glm::vec3 &specular = glm::vec3(1.0f), float intensity = 1.0f);

    void SetupShader(Shader &shader, int index) const override;
    void OnInspectorGUI() override;
    int getNum() const override
    {
        return number;
    }

    int number;
    static int count;

    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float intensity;
};

class SpotLight : public Light
{
  public:
    SpotLight(const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &direction = glm::vec3(1.0f, 0.0f, 0.0f),
              const glm::vec3 &ambient = glm::vec3(1.0f), const glm::vec3 &diffuse = glm::vec3(1.0f), const glm::vec3 &specular = glm::vec3(1.0f),
              float intensity = 1.0f, float cutOff = 12.5f, float outerCutOff = 17.5f);

    void SetupShader(Shader &shader, int index) const override;
    void OnInspectorGUI() override;

    int getNum() const override
    {
        return number;
    }
    int number;
    static int count;

    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float intensity;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};