#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <tuple>
#include <vector>


class Framebuffer
{
  public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    // 普通颜色纹理
    void AddColorTexture(GLint internalFormat, GLenum format, GLenum type);

    // 多重采样颜色纹理（MSAA）
    void AddColorTextureMultisample(GLint internalFormat, int samples);

    // 深度纹理（用于阴影、SSAO 等）
    void AddDepthTexture();

    // 深度/模板渲染缓冲（默认附件，兼容旧显卡）
    void AddDepthBuffer();

    // 检查完整性
    void CheckComplete();

    unsigned int GetID() const
    {
        return ID;
    }



    // 绑定/解绑
    void Bind() const;
    void Unbind() const;

    // 快捷绑定颜色纹理到指定 slot
    void BindTexture(unsigned int index, unsigned int slot) const;

    // 获取深度纹理
    unsigned int GetDepthTexture() const
    {
        return depthTexture;
    }

    // 获取颜色纹理
    unsigned int GetColorTexture(unsigned int index) const;

    // 尺寸
    int GetWidth() const
    {
        return width;
    }
    int GetHeight() const
    {
        return height;
    }

    // 调整分辨率（保留原始格式）
    void Resize(int newWidth, int newHeight);

    // 清理并设置背景色
    void Clear(const glm::vec4 &color = {0.0f, 0.0f, 0.0f, 1.0f});

    void reset()
    {
        if (ID != 0)
        {
            glDeleteFramebuffers(1, &ID);
            ID = 0;
        }
        colorTextures.clear();
        colorFormats.clear();
        colorSamples.clear();
        depthTexture = 0;
        depthBuffer = 0;
    }

  private:
    unsigned int ID = 0;
    int width, height;

    std::vector<unsigned int> colorTextures;
    std::vector<std::tuple<GLint, GLenum, GLenum>> colorFormats; // 用于重建
    std::vector<int> colorSamples;                               // 每个纹理的样本数（0 表示非 MSAA）

    unsigned int depthTexture = 0;
    unsigned int depthBuffer = 0;
};