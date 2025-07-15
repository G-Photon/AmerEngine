#include "core/Renderer.hpp"
#include "core/Geometry.hpp"
#include "utils/FileSystem.hpp"
#include <iostream>

Renderer::Renderer(int width, int height) : width(width), height(height)
{
}

Renderer::~Renderer()
{
    // 清理资源
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    for (auto &model : models)
    {
        model.reset();
    }
    for (auto &light : lights)
    {
        light.reset();
    }
    for (auto &primitive : primitives)
    {
        primitive.material.reset();
    }
    gBuffer.reset();
    shadowBuffer.reset();
    hdrBuffer.reset();
    bloomBuffer.reset();
    ssaoBuffer.reset();
    forwardShader.reset();
    deferredGeometryShader.reset();
    deferredLightingShader.reset();
    shadowDepthShader.reset();
    skyboxShader.reset();
    hdrShader.reset();
    bloomShader.reset();
    ssaoShader.reset();
    environmentMap.reset();
    mainCamera.reset();
}


void Renderer::BeginFrame()
{
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame()
{

}

void Renderer::Update(float deltaTime)
{
    // 更新相机
    mainCamera->Update(deltaTime);

    // // 更新光源
    // for (auto &light : lights)
    // {
    //     light->Update(deltaTime);
    // }

    // // 更新模型
    // for (auto &model : models)
    // {
    //     model->Update(deltaTime);
    // }

    // // 更新几何体
    // for (auto &primitive : primitives)
    // {
    //     primitive.Update(deltaTime);
    // }
}

void Renderer::Initialize()
{
    // 加载着色器
    forwardShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/forward/blinn_phong.vert"),
                                             FileSystem::GetPath("resources/shaders/forward/blinn_phong.frag"));

    deferredGeometryShader =
        std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/deferred/geometry_pass.vert"),
                                 FileSystem::GetPath("resources/shaders/deferred/geometry_pass.frag"));

    deferredLightingShader =
        std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/deferred/lighting_pass.vert"),
                                 FileSystem::GetPath("resources/shaders/deferred/lighting_pass.frag"));

    // 初始化帧缓冲
    SetupGBuffer();
    SetupShadowBuffer();
    SetupHDRBuffer();

    // 加载默认纹理
    auto whiteTexture = std::make_shared<Texture>();
    whiteTexture->CreateSolidColor(glm::vec3(1.0f));

    auto blackTexture = std::make_shared<Texture>();
    blackTexture->CreateSolidColor(glm::vec3(0.0f));

    auto normalTexture = std::make_shared<Texture>();
    normalTexture->CreateNormalMap();

    // 设置天空盒
    SetupSkybox();
}

void Renderer::SetupGBuffer()
{
    gBuffer = std::make_unique<Framebuffer>(width, height);

    // 位置、法线、反照率、金属度/粗糙度/ao
    gBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);      // 位置 + 深度
    gBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);      // 法线
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 反照率
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 金属度/粗糙度/ao

    gBuffer->AddDepthBuffer();
    gBuffer->CheckComplete();
}

void Renderer::SetupShadowBuffer()
{
    shadowBuffer = std::make_unique<Framebuffer>(2048, 2048);
    shadowBuffer->AddDepthTexture();
    shadowBuffer->CheckComplete();
}

void Renderer::SetupHDRBuffer()
{
    hdrBuffer = std::make_unique<Framebuffer>(width, height);
    hdrBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
    hdrBuffer->AddDepthBuffer();
    hdrBuffer->CheckComplete();

    bloomBuffer = std::make_unique<Framebuffer>(width, height);
    bloomBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
    bloomBuffer->CheckComplete();
}

void Renderer::SetupSkybox()
{
    float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
        -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,
        1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,
        1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,
        -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    skyboxShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/utility/skybox.vert"),
                                            FileSystem::GetPath("resources/shaders/utility/skybox.frag"));
}

void Renderer::RenderScene()
{
    if (shadowEnabled)
    {
        RenderShadows();
    }

    switch (currentMode)
    {
    case FORWARD:
        RenderForward();
        break;
    case DEFERRED:
        RenderDeferred();
        break;
    }

    if (hdrEnabled || bloomEnabled || ssaoEnabled)
    {
        RenderPostProcessing();
    }
}

void Renderer::RenderForward()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    forwardShader->Use();
    // 设置相机、光照等uniform
    glm::mat4 view = mainCamera->GetViewMatrix();
    glm::mat4 projection = mainCamera->GetProjectionMatrix(static_cast<float>(width) / height);
    forwardShader->SetMat4("view", view);
    forwardShader->SetMat4("projection", projection);
    forwardShader->SetVec3("viewPos", mainCamera->Position);
    forwardShader->SetBool("iblEnabled", iblEnabled);
    forwardShader->SetBool("pbrEnabled", pbrEnabled);
    forwardShader->SetBool("gammaCorrection", gammaCorrection);
    forwardShader->SetBool("shadowEnabled", shadowEnabled);
    forwardShader->SetBool("iblEnabled", iblEnabled);
    forwardShader->SetBool("hdrEnabled", hdrEnabled);
    forwardShader->SetBool("bloomEnabled", bloomEnabled);
    forwardShader->SetBool("ssaoEnabled", ssaoEnabled);
    // 渲染所有模型和几何体
    for (auto &model : models)
    {
        model->Draw(*forwardShader);
    }

    for (auto &primitive : primitives)
    {
        primitive.mesh->Draw(*forwardShader);
    }

    if (iblEnabled)
    {
        RenderSkybox();
    }
}

void Renderer::RenderDeferred()
{
    // 几何处理阶段
    gBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    deferredGeometryShader->Use();
    // 设置相机等uniform

    for (auto &model : models)
    {
        model->Draw(*deferredGeometryShader);
    }

    for (auto &primitive : primitives)
    {
        primitive.mesh->Draw(*deferredGeometryShader);
    }

    // 光照处理阶段
    glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer->GetID());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    deferredLightingShader->Use();
    // 绑定GBuffer纹理并设置uniform

    // 渲染全屏四边形
    glBindVertexArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (iblEnabled)
    {
        RenderSkybox();
    }
}

void Renderer::RenderShadows()
{
    shadowBuffer->Bind();
    glViewport(0, 0, shadowBuffer->GetWidth(), shadowBuffer->GetHeight());
    glClear(GL_DEPTH_BUFFER_BIT);

    shadowDepthShader->Use();
    // 设置光源VP矩阵

    for (auto &model : models)
    {
        model->Draw(*shadowDepthShader);
    }

    for (auto &primitive : primitives)
    {
        primitive.mesh->Draw(*shadowDepthShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}

void Renderer::RenderSkybox()
{
    glDepthFunc(GL_LEQUAL);
    skyboxShader->Use();
    // 设置视图矩阵(移除平移部分)
    glm::mat4 view = glm::mat4(glm::mat3(mainCamera->GetViewMatrix()));
    skyboxShader->SetMat4("view", view);
    skyboxShader->SetMat4("projection", mainCamera->GetProjectionMatrix(static_cast<float>(width) / height));

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap->GetID());
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void Renderer::CreatePrimitive(Geometry::Type type, const glm::vec3 &position, const glm::vec3 &scale,
                               const glm::vec3 &rotation, const Material &material)
{
    Geometry::Primitive primitive;
    primitive.type = type;
    primitive.position = position;
    primitive.scale = scale;
    primitive.rotation = rotation;
    primitive.material = std::make_shared<Material>(material);

    switch (type)
    {
    case Geometry::SPHERE:
        primitive.mesh = Geometry::CreateSphere();
        break;
    case Geometry::CUBE:
        primitive.mesh = Geometry::CreateCube();
        break;
    case Geometry::CYLINDER:
        primitive.mesh = Geometry::CreateCylinder();
        break;
    case Geometry::CONE:
        primitive.mesh = Geometry::CreateCone();
        break;
    case Geometry::PRISM:
        primitive.mesh = Geometry::CreatePrism();
        break;
    case Geometry::PYRAMID:
        primitive.mesh = Geometry::CreatePyramid();
        break;
    case Geometry::TORUS:
        primitive.mesh = Geometry::CreateTorus();
        break;
    case Geometry::ELLIPSOID:
        primitive.mesh = Geometry::CreateEllipsoid();
        break;
    default:
        std::cerr << "Unsupported geometry type!" << std::endl;
        return;
    }

    primitives.push_back(primitive);
}

void Renderer::RenderPostProcessing()
{
    // 后处理效果
    if (hdrEnabled)
    {
        hdrShader->Use();
        hdrShader->SetInt("scene", 0);
        hdrBuffer->BindTexture(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 渲染全屏四边形
    }
    if (bloomEnabled)
    {
        bloomShader->Use();
        bloomShader->SetInt("scene", 0);
        bloomBuffer->BindTexture(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 渲染全屏四边形
    }
    if (ssaoEnabled)
    {
        ssaoShader->Use();
        ssaoShader->SetInt("scene", 0);
        ssaoBuffer->BindTexture(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 渲染全屏四边形
    }
    if (gammaCorrection)
    {
        // 应用伽马校正
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    else
    {
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrBuffer->GetColorTexture(0));
    glDrawArrays(GL_TRIANGLES, 0, 6); // 渲染全屏四边形
}
// 其他方法实现...
std::shared_ptr<Model> Renderer::LoadModel(const std::string &path)
{
    auto model = std::make_shared<Model>(path);
    models.push_back(model);
    return model;
}

void Renderer::AddLight(const std::shared_ptr<Light> &light)
{
    lights.push_back(light);
}

void Renderer::SetGammaCorrection(bool enabled)
{
    gammaCorrection = enabled;
}

void Renderer::SetMSAA(bool enabled, int samples)
{
    msaaEnabled = enabled;
    if (enabled)
    {
        glEnable(GL_MULTISAMPLE);
        glSampleCoverage(samples, GL_TRUE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }
}

void Renderer::SetHDR(bool enabled)
{
    hdrEnabled = enabled;
}

void Renderer::SetBloom(bool enabled)
{
    bloomEnabled = enabled;
}

void Renderer::SetSSAO(bool enabled)
{
    ssaoEnabled = enabled;
}

void Renderer::SetShadow(bool enabled)
{
    shadowEnabled = enabled;
}

void Renderer::SetPBR(bool enabled)
{
    pbrEnabled = enabled;
}

void Renderer::SetIBL(bool enabled)
{
    iblEnabled = enabled;
}

void Renderer::SetRenderMode(RenderMode mode)
{
    currentMode = mode;
}

void Renderer::Resize(int newWidth, int newHeight)
{
    width = newWidth;
    height = newHeight;

    // 更新帧缓冲和着色器
    gBuffer->Resize(newWidth, newHeight);
    hdrBuffer->Resize(newWidth, newHeight);
    bloomBuffer->Resize(newWidth, newHeight);
    shadowBuffer->Resize(2048, 2048); // 阴影缓冲大小固定
    glViewport(0, 0, newWidth, newHeight);
}