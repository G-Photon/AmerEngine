#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <string>

class Shader;
class Framebuffer;
class Camera;
class Renderer;

/**
 * @class RenderPass
 * @brief 渲染通道的基类
 * 
 * 每个渲染通道（Geometry Pass / Lighting Pass / Bloom 等）都继承此基类
 * 提供统一的接口和生命周期管理
 */
class RenderPass
{
public:
    virtual ~RenderPass() = default;

    /**
     * @brief 初始化渲染通道
     * 创建所需的帧缓冲、着色器等资源
     */
    virtual void Initialize(Renderer* renderer) = 0;

    /**
     * @brief 清理通道资源
     */
    virtual void Cleanup() = 0;

    /**
     * @brief 执行渲染通道
     * @param renderer 渲染器指针
     * @param deltaTime 帧时间（秒）
     */
    virtual void Execute(Renderer* renderer, float deltaTime) = 0;

    /**
     * @brief 处理窗口大小改变
     * @param width 新的宽度
     * @param height 新的高度
     */
    virtual void OnResize(int width, int height) = 0;

    /**
     * @brief 获取通道名称（用于调试）
     */
    virtual const char* GetName() const = 0;

    /**
     * @brief 获取通道执行时间（毫秒）
     * 用于性能分析
     */
    float GetExecutionTime() const { return executionTime; }

protected:
    float executionTime = 0.0f; // 通道执行时间（毫秒）
};
