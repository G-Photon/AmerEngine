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

    enum BackgroundType
    {
        SKYBOX_CUBEMAP,     // 传统6面天空盒
        HDR_ENVIRONMENT     // HDR环境贴图
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

    // 背景/环境贴图管理（统一接口）
    void LoadBackgroundEnvironment(const std::string& name, const std::string& path, bool isHDR = true);
    void LoadBackgroundSkybox(const std::string& name, const std::vector<std::string>& faces);
    void SwitchBackground(const std::string& name);
    std::vector<std::string> GetAvailableBackgrounds() const;
    std::string GetCurrentBackgroundName() const { return currentEnvironmentName; }
    
    void SetSkybox(const std::shared_ptr<Texture> &texture); // 保留兼容性

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
    void SetIBL(bool enabled);
    void SetBackgroundType(BackgroundType type);
    BackgroundType GetBackgroundType() const { return backgroundType; }
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
    int GetMSAASamples() const
    {
        return msaaSamples;
    }
    bool IsFXAAEnabled() const
    {
        return fxaaEnabled;
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
    std::vector<Geometry::Primitive> &GetPrimitives()
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
        if (index < 0 || index >= models.size() + primitives.size() + pointLights.size() + directionalLights.size() +
                                      spotLights.size())
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
        else if (index <
                 models.size() + primitives.size() + pointLights.size() + directionalLights.size() + spotLights.size())
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

    void SetFXAA(bool enabled)
    {
        fxaaEnabled = enabled;
    }

    GLuint GetViewportTexture() const
    {
        return viewportBuffer->GetColorTexture(0); // 假设你有这个函数
    }

    void NewScene();
    void SaveScene(const std::string &path);
    void LoadScene(const std::string &path);

  private:
    void RenderForward();
    void RenderDeferred();
    void RenderPostProcessing();
    void RenderLights();
    void RenderQuad();
    void RenderCube();

    void SetupGBuffer();
    void SetupShadowBuffer();
    void SetupHDRBuffer();
    void SetupHDRBufferMS();
    void SetupBloomBuffer();
    void SetupSSAOBuffer();
    void SetupFXAABuffer();
    void SetupViewportBuffer();
    void SetupSkybox();

    void GenerateSSAOKernel();
    void GenerateSSAONoiseTexture();

    // 多光源阴影管理
    void CreateShadowBufferForLight(Light* light);
    void RemoveShadowBufferForLight(Light* light);
    unsigned int GetShadowBufferIndexForLight(Light* light);
    void ClearLightShadowBuffers(); // 清理所有光源阴影缓冲区

    void RenderSkybox();
    void RenderShadows();
    void RenderSSAO();
    
    // IBL相关方法
    void SetupIBL();
    void LoadHDREnvironment(const std::string& hdrPath);
    unsigned int LoadHDRTexture(const std::string& path);
    void GenerateIBLTextures();
    
    // 背景/环境贴图管理（统一）
    struct BackgroundData {
        std::string name;
        std::string path;
        BackgroundType type;
        unsigned int envCubemap = 0;
        unsigned int irradianceMap = 0;
        unsigned int prefilterMap = 0;
        bool isHDR = false;
    };
    
    void LoadSkyboxEnvironment(const std::string& name, const std::vector<std::string>& faces);
    void GenerateIBLForEnvironment(BackgroundData& envData);
    void InitializeDefaultBackgrounds();

    int width, height;

    RenderMode currentMode = FORWARD;

    // 帧缓冲
    std::unique_ptr<Framebuffer> gBuffer;
    std::unique_ptr<Framebuffer> shadowBuffer;  // 保留兼容性
    std::unique_ptr<Framebuffer> hdrBuffer;
    std::unique_ptr<Framebuffer> bloomPrefilterBuffer;
    std::unique_ptr<Framebuffer> bloomBlurBuffers[2];
    std::unique_ptr<Framebuffer> ssaoBuffer;
    std::unique_ptr<Framebuffer> ssaoBlurBuffer;
    std::unique_ptr<Framebuffer> fxaaBuffer;
    std::unique_ptr<Framebuffer> viewportBuffer; // 用于显示渲染结果

    // 多光源阴影缓冲区管理
    std::vector<std::unique_ptr<Framebuffer>> lightShadowBuffers;
    std::unordered_map<Light*, unsigned int> lightToShadowMap;

    std::unique_ptr<Framebuffer> hdrBufferMS;

    // 着色器
    std::unique_ptr<Shader> forwardShader;
    std::unique_ptr<Shader> pbrShader;
    std::unique_ptr<Shader> deferredGeometryShader;
    std::unique_ptr<Shader> deferredLightingShader;
    std::unique_ptr<Shader> pbrDeferredGeometryShader;
    std::unique_ptr<Shader> pbrDeferredLightingShader;
    std::unique_ptr<Shader> shadowDepthShader;
    std::unique_ptr<Shader> skyboxShader;
    std::unique_ptr<Shader> hdrShader;
    std::unique_ptr<Shader> bloomPreShader;
    std::unique_ptr<Shader> bloomBlurShader;
    std::unique_ptr<Shader> ssaoShader;
    std::unique_ptr<Shader> ssaoBlurShader;
    std::unique_ptr<Shader> lightsShader;
    std::unique_ptr<Shader> postProcessShader;
    std::unique_ptr<Shader> postShaderMS; // 采样 sampler2DMS
    std::unique_ptr<Shader> fxaaShader;
    
    // IBL着色器
    std::unique_ptr<Shader> equirectangularToCubemapShader;
    std::unique_ptr<Shader> irradianceShader;
    std::unique_ptr<Shader> prefilterShader;
    std::unique_ptr<Shader> brdfShader;

    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;

    // 场景数据
    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<PointLight>> pointLights;
    std::vector<std::shared_ptr<DirectionalLight>> directionalLights;
    std::vector<std::shared_ptr<SpotLight>> spotLights;
    std::vector<Geometry::Primitive> primitives;

    // 环境
    std::shared_ptr<Texture> environmentMap; // 保留兼容性
    
    // IBL纹理
    unsigned int envCubemap;
    unsigned int irradianceMap;
    unsigned int prefilterMap;
    unsigned int brdfLUTTexture;
    std::unique_ptr<Framebuffer> iblCaptureBuffer; // 用于IBL预计算
    unsigned int cubeVAO=0, cubeVBO=0;
    
    // 背景/环境贴图管理（统一）
    std::unordered_map<std::string, BackgroundData> backgrounds;
    std::string currentEnvironmentName;

    // 相机
    std::shared_ptr<Camera> mainCamera;

    std::vector<glm::vec3> ssaoKernel; // SSAO采样核心
    GLuint ssaoNoiseTexture;           // SSAO旋转噪声纹理
    unsigned int ssaoKernelSize = 64;  // SSAO采样核心大小
    unsigned int ssaoNoiseSize = 4;    // SSAO噪声纹理尺寸

    // 特效状态
    bool gammaCorrection = false;
    bool msaaEnabled = false;
    int msaaSamples = 4; // MSAA采样倍数
    bool hdrEnabled = false;
    bool bloomEnabled = false;
    bool ssaoEnabled = false;
    bool shadowEnabled = false;
    bool iblEnabled = false;
    bool showLights = false;
    bool fxaaEnabled = false;
    
    // 背景类型
    BackgroundType backgroundType = SKYBOX_CUBEMAP;
};