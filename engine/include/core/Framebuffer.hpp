#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
class Framebuffer
{
  public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    
    void AddColorTexture(GLint internalFormat, GLenum format, GLenum type);
    void AddDepthTexture();
    void AddDepthBuffer();

    void CheckComplete();

    void Bind() const;
    void Unbind() const;

    unsigned int GetID() const
    {
        return ID;
    }
    unsigned int GetColorTexture(int index = 0) const
    {
        return colorTextures[index];
    }
    unsigned int GetDepthTexture() const
    {
        return depthTexture;
    }
    int GetWidth() const
    {
        return width;
    }
    int GetHeight() const
    {
        return height;
    }

    void BindTexture(int index = 0) const
    {
        glBindTexture(GL_TEXTURE_2D, colorTextures[index]);
    }
    void Resize(int newWidth, int newHeight);
    void Clear(const glm::vec4 &color = glm::vec4(0.0f));
  private:
    unsigned int ID;
    std::vector<unsigned int> colorTextures;
    unsigned int depthTexture = 0;
    unsigned int depthBuffer = 0;
    int width, height;
};