#pragma once

#include "Shader.hpp"
#include "glm/fwd.hpp"
#include <imgui.h>

class Light
{
  public:
    virtual ~Light() = default;
    virtual void SetupShader(Shader &shader, int index = 0) const = 0;
    virtual int getNum() const
    {
        return 1; // 默认返回1，子类可以重载此方法
    }

    virtual void RemoveFromScene(int index = 0)
    {
        // 默认实现为空，子类可以重载此方法
    }

    virtual glm::vec3 getPosition()=0;

    virtual glm::vec3 getLightColor() const
    {
        return glm::vec3(1.0f); // 默认返回白色光
    }

    virtual int getType() const
    {
        return 0; // 默认返回0，子类可以重载此方法
    }
};

class PointLight : public Light
{
  public:
    PointLight(const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &ambient = glm::vec3(1.0f), const glm::vec3 &diffuse = glm::vec3(1.0f), const glm::vec3 &specular = glm::vec3(1.0f), float intensity = 1.0f);

    void SetupShader(Shader &shader, int index) const override;
    int getNum() const override
    {
        return number;
    }

    glm::vec3 getPosition() override
    {
        return position;
    }

    glm::vec3 getLightColor() const override
    {
        return diffuse;
    }

    int getType() const override
    {
        return 0; // 点光源类型
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
    int getNum() const override
    {
        return number;
    }

    glm::vec3 getPosition() override
    {
        return glm::vec3(0.0f); // 定向光没有位置，返回零向量
    }

    glm::vec3 getLightColor() const override
    {
        return diffuse;
    }

    int getType() const override
    {
        return 1; // 定向光类型
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

    int getNum() const override
    {
        return number;
    }

    glm::vec3 getPosition() override
    {
        return position;
    }
    glm::vec3 getLightColor() const override
    {
        return diffuse;
    }

    int getType() const override
    {
        return 2; // 聚光灯类型
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