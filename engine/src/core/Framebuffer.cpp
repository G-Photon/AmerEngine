#include "core/Framebuffer.hpp"

Framebuffer::Framebuffer(int width, int height) : width(width), height(height)
{
    glGenFramebuffers(1, &ID);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &ID);

    if (!colorTextures.empty())
        glDeleteTextures(colorTextures.size(), colorTextures.data());

    if (depthTexture)
        glDeleteTextures(1, &depthTexture);

    if (depthBuffer)
        glDeleteRenderbuffers(1, &depthBuffer);
}

void Framebuffer::AddColorTexture(GLint internalFormat, GLenum format, GLenum type)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorTextures.size(), GL_TEXTURE_2D, tex, 0);

    colorTextures.push_back(tex);
    colorFormats.emplace_back(internalFormat, format, type);
    colorSamples.push_back(0); // 非 MSAA
}

void Framebuffer::AddColorTextureMultisample(GLint internalFormat, int samples)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorTextures.size(), GL_TEXTURE_2D_MULTISAMPLE, tex,
                           0);

    colorTextures.push_back(tex);
    colorFormats.emplace_back(internalFormat, GL_RGBA, GL_FLOAT); // 占位
    colorSamples.push_back(samples);
}

void Framebuffer::AddDepthTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
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

    if (!colorTextures.empty())
    {
        std::vector<GLenum> attachments;
        for (size_t i = 0; i < colorTextures.size(); ++i)
            attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
        glDrawBuffers(attachments.size(), attachments.data());
    }
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Framebuffer is not complete!");

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

void Framebuffer::BindTexture(unsigned int index, unsigned int slot) const
{
    if (index >= colorTextures.size())
        return;
    glActiveTexture(GL_TEXTURE0 + slot);
    GLenum target = colorSamples[index] > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    glBindTexture(target, colorTextures[index]);
}

unsigned int Framebuffer::GetColorTexture(unsigned int index) const
{
    return (index < colorTextures.size()) ? colorTextures[index] : 0;
}

void Framebuffer::Resize(int newWidth, int newHeight)
{
    width = newWidth;
    height = newHeight;

    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    // 清理旧资源
    if (!colorTextures.empty())
        glDeleteTextures(colorTextures.size(), colorTextures.data());
    colorTextures.clear();
    bool hadDepthTex = depthTexture != 0;
    bool hadDepthBuf = depthBuffer != 0;
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
    auto remaincolorFormats = colorFormats;
    auto remaincolorSamples = colorSamples;
    colorFormats.clear();
    colorSamples.clear();

    // 按原始格式重建颜色纹理
    for (size_t i = 0; i < remaincolorFormats.size(); ++i)
    {
        auto [internal, format, type] = remaincolorFormats[i];
        int samples = remaincolorSamples[i];
        if (samples > 0)
            AddColorTextureMultisample(internal, samples);
        else
            AddColorTexture(internal, format, type);
    }

    // 重建深度附件（根据之前是否存在决定）
    if (hadDepthTex)
        AddDepthTexture();
    if (hadDepthBuf)
        AddDepthBuffer();

    CheckComplete();
}

void Framebuffer::Clear(const glm::vec4 &color)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

