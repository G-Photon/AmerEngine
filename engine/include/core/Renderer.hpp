#pragma once

#include "Camera.hpp"
#include "Framebuffer.hpp"
#include "Geometry.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

class Renderer
{
  public:
    enum RenderMode
    {
        FORWARD,
        DEFERRED
    };

    Renderer(int width, int height);
    ~Renderer();

    void Initialize();
    void Resize(int width, int height);

    void BeginFrame();
    void EndFrame();

    void RenderScene();
    void RenderUI();

    // 着色器管理
    void LoadShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath);
    std::shared_ptr<Shader> GetShader(const std::string &name) const;
    void UseShader(const std::shared_ptr<Shader> &shader);

    void UseShader(const std::string &name);
    void SetGlobalUniforms(const Camera &camera);
    void SetEnvironmentMap(const std::shared_ptr<Texture> &texture);
    void SetSkybox(const std::shared_ptr<Texture> &texture);

    // 更新
    void Update(float deltaTime);

    // 帧缓冲管理
    void CreateFramebuffer(const std::string &name, int width, int height, bool depth = true);
    std::shared_ptr<Framebuffer> GetFramebuffer(const std::string &name) const;
    void BindFramebuffer(const std::string &name);
    void UnbindFramebuffer();
    void ClearFramebuffer(const std::string &name, const glm::vec4 &color = glm::vec4(0.0f));

    // 几何体创建
    void CreatePrimitive(Geometry::Type type, const glm::vec3 &position, const glm::vec3 &scale,
                         const glm::vec3 &rotation, const Material &material);

    // 模型加载
    std::shared_ptr<Model> LoadModel(const std::string &path);

    // 光源管理
    void AddLight(const std::shared_ptr<Light> &light);
    std::vector<std::shared_ptr<Light>> GetLights() const
    {
        std::vector<std::shared_ptr<Light>> lights;
        lights.insert(lights.end(), pointLights.begin(), pointLights.end());
        lights.insert(lights.end(), directionalLights.begin(), directionalLights.end());
        lights.insert(lights.end(), spotLights.begin(), spotLights.end());
        return lights;
    }

    // 特效开关
    void SetGammaCorrection(bool enabled);
    void SetMSAA(bool enabled, int samples = 4);
    void SetHDR(bool enabled);
    void SetBloom(bool enabled);
    void SetSSAO(bool enabled);
    void SetShadow(bool enabled);
    void SetPBR(bool enabled);
    void SetIBL(bool enabled);
    void SetLightsEnabled(bool enabled)
    {
        showLights = enabled;
    }

    // 渲染模式
    void SetRenderMode(RenderMode mode);
    RenderMode GetRenderMode() const
    {
        return currentMode;
    }

    // 状态查询
    bool IsGammaCorrectionEnabled() const
    {
        return gammaCorrection;
    }
    bool IsMSAAEnabled() const
    {
        return msaaEnabled;
    }
    bool IsHDREnabled() const
    {
        return hdrEnabled;
    }
    bool IsBloomEnabled() const
    {
        return bloomEnabled;
    }
    bool IsSSAOEnabled() const
    {
        return ssaoEnabled;
    }
    bool IsShadowEnabled() const
    {
        return shadowEnabled;
    }
    bool IsPBREnabled() const
    {
        return pbrEnabled;
    }
    bool IsIBLEnabled() const
    {
        return iblEnabled;
    }
    bool IsLightsEnabled() const
    {
        return showLights;
    }

    // 场景对象访问
    const std::vector<std::shared_ptr<Model>> &GetModels() const
    {
        return models;
    }
    const std::vector<Geometry::Primitive> &GetPrimitives() const
    {
        return primitives;
    }
    size_t GetModelCount() const
    {
        return models.size();
    }
    std::shared_ptr<Model> GetModel(int index) const
    {
        return models[index];
    }

    std::shared_ptr<Camera> GetCamera() const
    {
        return mainCamera;
    }

    void DeleteObject(int index)
    {
        if (index < 0 || index >= models.size() + primitives.size()+ pointLights.size() + directionalLights.size() + spotLights.size())
            return;

        if (index < models.size())
        {
            models.erase(models.begin() + index);
        }
        else if (index < models.size() + primitives.size())
        {
            // 删除几何体
            index -= models.size();
            primitives.erase(primitives.begin() + index);
        }
        else if (index < models.size() + primitives.size() + pointLights.size() + directionalLights.size() + spotLights.size())
        {
            // 删除光源
            index -= (models.size() + primitives.size());
            // 依次检查并删除对应类型的光源
            if (index < pointLights.size())
            {
                pointLights.erase(pointLights.begin() + index);
            }
            else if (index < pointLights.size() + directionalLights.size())
            {
                index -= pointLights.size();
                directionalLights.erase(directionalLights.begin() + index);
            }
            else if (index < pointLights.size() + directionalLights.size() + spotLights.size())
            {
                index -= (pointLights.size() + directionalLights.size());
                spotLights.erase(spotLights.begin() + index);
            }
        }
        return;
    }

    std::vector<std::shared_ptr<Material>> getALLMaterials() const
    {
        std::vector<std::shared_ptr<Material>> allMaterials;
        for (const auto &model : models)
        {
            for (const auto &mesh : model->GetMeshes())
            {
                allMaterials.push_back(mesh->GetMaterial());
            }
        }
        for (const auto &primitive : primitives)
        {
            allMaterials.push_back(primitive.mesh->GetMaterial());
        }
        return allMaterials;
    }
    
  private:
    void RenderForward();
    void RenderDeferred();
    void RenderPostProcessing();
    void RenderLights();

    void SetupGBuffer();
    void SetupShadowBuffer();
    void SetupHDRBuffer();
    void SetupSkybox();

    void RenderSkybox();
    void RenderShadows();

    int width, height;

    RenderMode currentMode = FORWARD;

    // 帧缓冲
    std::unique_ptr<Framebuffer> gBuffer;
    std::unique_ptr<Framebuffer> shadowBuffer;
    std::unique_ptr<Framebuffer> hdrBuffer;
    std::unique_ptr<Framebuffer> bloomBuffer;
    std::unique_ptr<Framebuffer> ssaoBuffer;

    // 着色器
    std::unique_ptr<Shader> forwardShader;
    std::unique_ptr<Shader> deferredGeometryShader;
    std::unique_ptr<Shader> deferredLightingShader;
    std::unique_ptr<Shader> shadowDepthShader;
    std::unique_ptr<Shader> skyboxShader;
    std::unique_ptr<Shader> hdrShader;
    std::unique_ptr<Shader> bloomShader;
    std::unique_ptr<Shader> ssaoShader;
    std::unique_ptr<Shader> lightsShader;

    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;

    // 场景数据
    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<PointLight>> pointLights;
    std::vector<std::shared_ptr<DirectionalLight>> directionalLights;
    std::vector<std::shared_ptr<SpotLight>> spotLights;
    std::vector<Geometry::Primitive> primitives;

    // 环境
    std::shared_ptr<Texture> environmentMap;
    unsigned int skyboxVAO, skyboxVBO;

    // 相机
    std::shared_ptr<Camera> mainCamera;

    // 特效状态
    bool gammaCorrection = false;
    bool msaaEnabled = false;
    bool hdrEnabled = false;
    bool bloomEnabled = false;
    bool ssaoEnabled = false;
    bool shadowEnabled = false;
    bool pbrEnabled = false;
    bool iblEnabled = false;
    bool showLights = false;
};