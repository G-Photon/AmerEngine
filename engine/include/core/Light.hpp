#pragma once

#include "Shader.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Geometry.hpp"
#include <imgui.h>
#include <memory>

class Light
{
  public:
    virtual ~Light() = default;
    virtual void SetupShader(Shader &shader, int index = 0, bool globalShadowEnabled = true) const = 0;
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

    virtual void drawLightMesh(const std::unique_ptr<Shader> &shader)
    {
        // 默认实现为空，子类可以重载此方法
    }

    // 阴影相关方法
    virtual bool HasShadows() const { return false; }
    virtual glm::mat4 GetLightSpaceMatrix() const { return glm::mat4(1.0f); }
    virtual unsigned int GetShadowMap() const { return 0; }
    virtual void SetShadowMap(unsigned int shadowMap) {}
    virtual void SetShadowEnabled(bool enabled) {}
    virtual bool IsShadowEnabled() const { return false; }
    virtual int GetShadowMapIndex() const { return -1; } // 获取阴影贴图索引
};

class PointLight : public Light
{
  public:
    PointLight(const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &ambient = glm::vec3(1.0f), const glm::vec3 &diffuse = glm::vec3(1.0f), const glm::vec3 &specular = glm::vec3(1.0f), float intensity = 1.0f);

    void SetupShader(Shader &shader, int index = 0, bool globalShadowEnabled = true) const override;
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

    void drawLightMesh(const std::unique_ptr<Shader> &shader) override;

    // 阴影相关方法
    bool HasShadows() const override { return shadowEnabled; }
    glm::mat4 GetLightSpaceMatrix() const override;
    unsigned int GetShadowMap() const override { return shadowMap; }
    void SetShadowMap(unsigned int shadowMap) override { this->shadowMap = shadowMap; }
    void SetShadowEnabled(bool enabled) override { shadowEnabled = enabled; }
    bool IsShadowEnabled() const override { return shadowEnabled; }
    int GetShadowMapIndex() const override { return number; }

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

    // 阴影参数
    bool shadowEnabled = false;
    unsigned int shadowMap = 0;
    float shadowNearPlane = 0.1f;
    float shadowFarPlane = 100.0f;
};

class DirectionalLight : public Light
{
  public:
    DirectionalLight(const glm::vec3 &direction = glm::vec3(-0.2f, -1.0f, -0.3f),
                     const glm::vec3 &ambient = glm::vec3(1.0f), const glm::vec3 &diffuse = glm::vec3(1.0f), const glm::vec3 &specular = glm::vec3(1.0f), float intensity = 1.0f);

    void SetupShader(Shader &shader, int index = 0, bool globalShadowEnabled = true) const override;
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
    void drawLightMesh(const std::unique_ptr<Shader> &shader) override;

    // 阴影相关方法
    bool HasShadows() const override { return shadowEnabled; }
    glm::mat4 GetLightSpaceMatrix() const override;
    unsigned int GetShadowMap() const override { return shadowMap; }
    void SetShadowMap(unsigned int shadowMap) override { this->shadowMap = shadowMap; }
    void SetShadowEnabled(bool enabled) override { shadowEnabled = enabled; }
    bool IsShadowEnabled() const override { return shadowEnabled; }
    int GetShadowMapIndex() const override { return number; }

    int number;
    static int count;

    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float intensity;

    // 阴影参数
    bool shadowEnabled = false;
    unsigned int shadowMap = 0;
    float shadowNearPlane = 0.1f;
    float shadowFarPlane = 100.0f;
    float shadowOrthoSize = 20.0f;
};

class SpotLight : public Light
{
  public:
    SpotLight(const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &direction = glm::vec3(1.0f, 0.0f, 0.0f),
              const glm::vec3 &ambient = glm::vec3(1.0f), const glm::vec3 &diffuse = glm::vec3(1.0f), const glm::vec3 &specular = glm::vec3(1.0f),
              float intensity = 1.0f, float cutOff = 12.5f, float outerCutOff = 17.5f);

    void SetupShader(Shader &shader, int index = 0, bool globalShadowEnabled = true) const override;

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

    void drawLightMesh(const std::unique_ptr<Shader> &shader) override;

    // 阴影相关方法
    bool HasShadows() const override { return shadowEnabled; }
    glm::mat4 GetLightSpaceMatrix() const override;
    unsigned int GetShadowMap() const override { return shadowMap; }
    void SetShadowMap(unsigned int shadowMap) override { this->shadowMap = shadowMap; }
    void SetShadowEnabled(bool enabled) override { shadowEnabled = enabled; }
    bool IsShadowEnabled() const override { return shadowEnabled; }
    int GetShadowMapIndex() const override { return number; }

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

    // 阴影参数
    bool shadowEnabled = false;
    unsigned int shadowMap = 0;
    float shadowNearPlane = 0.1f;
    float shadowFarPlane = 100.0f;
};