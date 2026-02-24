#pragma once

#include "core/RenderPass.hpp"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Shader;
class Framebuffer;
class Model;
class Mesh;

/**
 * @struct RenderCommand
 * @brief 单个渲染命令（用于排序和合批）
 */
struct RenderCommand
{
    std::shared_ptr<Shader> shader;           // 使用的着色器
    std::shared_ptr<Model> model;             // 所属模型
    std::shared_ptr<Mesh> mesh;               // 网格
    glm::mat4 modelMatrix;                    // 模型矩阵
    int materialType;                         // 材质类型（PBR / Blinn-Phong）
    uint64_t sortKey;                         // 排序键（用于排序）

    /**
     * @brief 计算排序键
     * 低优先级：shader指针 -> 高优先级：距离
     */
    void ComputeSortKey(const glm::vec3& cameraPos)
    {
        // 高32位存储着色器ID，低32位存储距离
        uint64_t shaderID = reinterpret_cast<uintptr_t>(shader.get()) & 0xFFFFFFFF;
        uint32_t distanceKey = 0;

        if (model)
        {
            float dist = glm::length(cameraPos - model->GetPosition());
            distanceKey = static_cast<uint32_t>(std::min(dist * 100.0f, 4294967295.0f));
        }

        sortKey = (shaderID << 32) | distanceKey;
    }
};

/**
 * @class DeferredGeometryPass
 * @brief 延迟渲染的几何通道
 * 
 * 输出G-Buffer，包含Position、Normal、Albedo等信息
 */
class DeferredGeometryPass : public RenderPass
{
public:
    void Initialize(Renderer* renderer) override;
    void Cleanup() override;
    void Execute(Renderer* renderer, float deltaTime) override;
    void OnResize(int width, int height) override;
    const char* GetName() const override { return "DeferredGeometryPass"; }

private:
    std::shared_ptr<Framebuffer> gBuffer;
    std::shared_ptr<Shader> blinnPhongShader;
    std::shared_ptr<Shader> pbrShader;

    /**
     * @brief 收集所有需要渲染的几何体
     * 执行视锥剔除，并排序
     */
    void CollectRenderCommands(Renderer* renderer, std::vector<RenderCommand>& outCommands);

    /**
     * @brief 执行批量渲染
     */
    void ExecuteBatch(const std::vector<RenderCommand>& commands, Renderer* renderer);
};

/**
 * @class DeferredLightingPass
 * @brief 延迟渲染的光照通道
 * 
 * 基于G-Buffer进行光照计算
 */
class DeferredLightingPass : public RenderPass
{
public:
    void Initialize(Renderer* renderer) override;
    void Cleanup() override;
    void Execute(Renderer* renderer, float deltaTime) override;
    void OnResize(int width, int height) override;
    const char* GetName() const override { return "DeferredLightingPass"; }

private:
    std::shared_ptr<Shader> lightingShader;
};

/**
 * @class PostProcessPass
 * @brief 后处理通道
 * 
 * 包含HDR、Bloom、SSAO、FXAA等后处理效果
 */
class PostProcessPass : public RenderPass
{
public:
    void Initialize(Renderer* renderer) override;
    void Cleanup() override;
    void Execute(Renderer* renderer, float deltaTime) override;
    void OnResize(int width, int height) override;
    const char* GetName() const override { return "PostProcessPass"; }

private:
    std::shared_ptr<Shader> bloomPrefilterShader;
    std::shared_ptr<Shader> bloomBlurShader;
    std::shared_ptr<Shader> ssaoShader;
    std::shared_ptr<Shader> ssaoBlurShader;
    std::shared_ptr<Shader> fxaaShader;
    std::shared_ptr<Shader> postProcessShader;
};

/**
 * @class ShadowPass
 * @brief 阴影贴图生成通道
 */
class ShadowPass : public RenderPass
{
public:
    void Initialize(Renderer* renderer) override;
    void Cleanup() override;
    void Execute(Renderer* renderer, float deltaTime) override;
    void OnResize(int width, int height) override;
    const char* GetName() const override { return "ShadowPass"; }
};
