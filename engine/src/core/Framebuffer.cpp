#include "core/Framebuffer.hpp"
#include <stdexcept>

Framebuffer::Framebuffer(int width, int height) : width(width), height(height)
{
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &ID);
    glDeleteTextures(colorTextures.size(), colorTextures.data());
    if (depthTexture)
    {
        glDeleteTextures(1, &depthTexture);
    }
    if (depthBuffer)
    {
        glDeleteRenderbuffers(1, &depthBuffer);
    }
}

void Framebuffer::AddColorTexture(GLint internalFormat, GLenum format, GLenum type)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorTextures.size(), GL_TEXTURE_2D, texture, 0);
    colorTextures.push_back(texture);
}

void Framebuffer::AddDepthTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    // 显式告诉OpenGL我们不使用颜色缓冲
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}

void Framebuffer::AddDepthBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
}

void Framebuffer::CheckComplete()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    if (colorTextures.size() > 0)
    {
        std::vector<unsigned int> attachments;
        for (size_t i = 0; i < colorTextures.size(); ++i)
        {
            attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
        }
        glDrawBuffers(attachments.size(), attachments.data());
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    glViewport(0, 0, width, height);
}

void Framebuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(int newWidth, int newHeight)
{
    width = newWidth;
    height = newHeight;

    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    for (auto &texture : colorTextures)
    {
        glDeleteTextures(1, &texture);
    }
    colorTextures.clear();
    
    if (depthTexture)
    {
        glDeleteTextures(1, &depthTexture);
        depthTexture = 0;
    }
    
    if (depthBuffer)
    {
        glDeleteRenderbuffers(1, &depthBuffer);
        depthBuffer = 0;
    }

    // Recreate textures
    AddColorTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    AddDepthTexture();
}

void Framebuffer::Clear(const glm::vec4 &color)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}