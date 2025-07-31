#include "core/Renderer.hpp"
#include "core/Camera.hpp"
#include "core/Framebuffer.hpp"
#include "core/Geometry.hpp"
#include "core/Light.hpp"
#include "core/Shader.hpp"
#include "core/Texture.hpp"
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
    for (auto &pointLight : pointLights)
    {
        pointLight.reset();
    }
    for (auto &directionalLight : directionalLights)
    {
        directionalLight.reset();
    }
    for (auto &spotLight : spotLights)
    {
        spotLight.reset();
    }
    for (auto &primitive : primitives)
    {
        primitive.mesh->SetMaterial(nullptr);
        primitive.mesh.reset();
    }
    gBuffer.reset();
    shadowBuffer.reset();
    hdrBuffer.reset();
    hdrBufferMS.reset();
    bloomPrefilterBuffer.reset();
    for (auto &blurBuffer : bloomBlurBuffers)
    {
        blurBuffer.reset();
    }
    ssaoBuffer.reset();
    forwardShader.reset();
    deferredGeometryShader.reset();
    deferredLightingShader.reset();
    shadowDepthShader.reset();
    skyboxShader.reset();
    hdrShader.reset();
    postProcessShader.reset();
    postShaderMS.reset();
    bloomPreShader.reset();
    bloomBlurShader.reset();
    ssaoShader.reset();
    environmentMap.reset();
    mainCamera.reset();
}

void Renderer::BeginFrame()
{
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

    lightsShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/utility/light.vert"),
                                            FileSystem::GetPath("resources/shaders/utility/light.frag"));

    postProcessShader = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/postprocess/quad.vert"),
                                                 FileSystem::GetPath("resources/shaders/postprocess/post.frag"));
    postShaderMS = std::make_unique<Shader>(FileSystem::GetPath("resources/shaders/postprocess/quad.vert"),
                                            FileSystem::GetPath("resources/shaders/postprocess/post_msaa.frag"));

    ssaoShader =
        std::make_unique<Shader>("resources/shaders/postprocess/quad.vert", "resources/shaders/postprocess/ssao.frag");
    bloomPreShader = std::make_unique<Shader>("resources/shaders/postprocess/quad.vert",
                                              "resources/shaders/postprocess/bloom_prefilter.frag");
    bloomBlurShader = std::make_unique<Shader>("resources/shaders/postprocess/quad.vert",
                                               "resources/shaders/postprocess/bloom_blur.frag");

    // 初始化帧缓冲
    SetupShadowBuffer();
    SetupGBuffer();
    SetupHDRBuffer();
    SetupHDRBufferMS();
    SetupBloomBuffer();
    SetupSSAOBuffer();

    // 加载默认纹理
    auto whiteTexture = std::make_shared<Texture>();
    whiteTexture->CreateSolidColor(glm::vec3(1.0f));

    auto blackTexture = std::make_shared<Texture>();
    blackTexture->CreateSolidColor(glm::vec3(0.0f));

    auto normalTexture = std::make_shared<Texture>();
    normalTexture->CreateNormalMap();

    environmentMap = std::make_shared<Texture>();
    environmentMap->LoadCubemap({FileSystem::GetPath("resources/textures/skybox/right.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/left.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/top.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/bottom.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/front.jpg"),
                                 FileSystem::GetPath("resources/textures/skybox/back.jpg")});

    // 设置天空盒
    SetupSkybox();

    // 设置相机
    mainCamera = std::make_shared<Camera>();
}

void Renderer::SetupGBuffer()
{
    gBuffer = std::make_unique<Framebuffer>(width, height);

    // 位置、法线、反照率、金属度/粗糙度/ao、漫反射/镜面反射贴图
    gBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);      // 位置 + 深度
    gBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);      // 法线
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 漫反射贴图
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 镜面反射贴图
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 金属度
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 粗糙度
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // AO
    gBuffer->AddColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); // 环境光贴图

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
}
void Renderer::SetupHDRBufferMS()
{
    hdrBufferMS = std::make_unique<Framebuffer>(width, height);
    hdrBufferMS->AddColorTextureMultisample(GL_RGBA16F, 4); // 使用4倍多重采样
    hdrBufferMS->AddDepthBufferMultisample();
    hdrBufferMS->CheckComplete();
}

void Renderer::SetupBloomBuffer()
{
    bloomPrefilterBuffer = std::make_unique<Framebuffer>(width, height);
    bloomPrefilterBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
    bloomPrefilterBuffer->CheckComplete();

    for (int i = 0; i < 2; ++i)
    {
        bloomBlurBuffers[i] = std::make_unique<Framebuffer>(width / (1 << i), height / (1 << i));
        bloomBlurBuffers[i]->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
        bloomBlurBuffers[i]->CheckComplete();
    }
}

void Renderer::SetupSSAOBuffer()
{
    ssaoBuffer = std::make_unique<Framebuffer>(width, height);
    ssaoBuffer->AddColorTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
    ssaoBuffer->CheckComplete();
}

void Renderer::SetupSkybox()
{
    float skyboxVertices[] = {// positions
                              -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                              -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                              1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                              -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

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

    if (showLights)
    {
        // 渲染光源
        RenderLights();
    }

    RenderPostProcessing();
}

void Renderer::RenderForward()
{
    if (msaaEnabled)
    {
        hdrBufferMS->Bind();
    }
    else
    {
        hdrBuffer->Bind();
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    forwardShader->Use();
    // 设置相机、光照等uniform
    glm::mat4 view = mainCamera->GetViewMatrix();
    glm::mat4 projection = mainCamera->GetProjectionMatrix(static_cast<float>(width) / height);
    forwardShader->SetMat4("view", view);
    forwardShader->SetMat4("projection", projection);
    forwardShader->SetVec3("viewPos", mainCamera->Position);

    // 设置光源
    forwardShader->SetInt("numLights[0]", directionalLights.size()); // 方向光数量
    forwardShader->SetInt("numLights[1]", pointLights.size());       // 点光
    forwardShader->SetInt("numLights[2]", spotLights.size());        // 聚光灯数量
    for (size_t i = 0; i < pointLights.size(); ++i)
    {
        pointLights[i]->SetupShader(*forwardShader, i);
    }
    for (size_t i = 0; i < directionalLights.size(); ++i)
    {
        directionalLights[i]->SetupShader(*forwardShader, i);
    }
    for (size_t i = 0; i < spotLights.size(); ++i)
    {
        spotLights[i]->SetupShader(*forwardShader, i);
    }
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    deferredGeometryShader->Use();

    // 设置相机等uniform
    deferredGeometryShader->SetMat4("view", mainCamera->GetViewMatrix());
    deferredGeometryShader->SetMat4("projection", mainCamera->GetProjectionMatrix(static_cast<float>(width) / height));
    deferredGeometryShader->SetVec3("viewPos", mainCamera->Position);

    for (auto &model : models)
    {
        model->Draw(*deferredGeometryShader);
    }

    for (auto &primitive : primitives)
    {
        primitive.mesh->Draw(*deferredGeometryShader);
    }

    // 光照处理阶段
    GLuint targetFBO = msaaEnabled ? hdrBufferMS->GetID() : hdrBuffer->GetID();
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->GetID());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glDepthMask(GL_FALSE);     // 不写入深度
    glEnable(GL_STENCIL_TEST); // 全程开模板
    glDisable(GL_CULL_FACE);   // 由每个 pass 自己决定

    deferredLightingShader->Use();
    // 绑定GBuffer纹理并设置uniform

    // 设置相机
    deferredLightingShader->SetMat4("view", mainCamera->GetViewMatrix());
    deferredLightingShader->SetMat4("projection", mainCamera->GetProjectionMatrix(static_cast<float>(width) / height));
    deferredLightingShader->SetVec3("viewPos", mainCamera->Position);
    deferredLightingShader->SetVec2("screenSize", glm::vec2(width, height));

    const int texSlots[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    const char *texNames[8] = {"gPosition", "gNormal", "gAlbedo", "gSpecular", "gMetallic", "gRoughness", "gAo", "gAmbient"};
    for (int i = 0; i < 8; ++i)
    {
        deferredLightingShader->SetInt(texNames[i], texSlots[i]);
        gBuffer->BindTexture(i, texSlots[i]);
    }

    for (const auto &light : GetLights())
    {
        if (light->getType() != 1)
        {
            // === 第一步：标记光体积区域 ===
            glClear(GL_STENCIL_BUFFER_BIT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // 不写颜色
            glDepthMask(GL_FALSE);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE); // 正背面都要

            glStencilFunc(GL_ALWAYS, 0, 0);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

            // 绘制光体积（仅更新模板缓冲区）
            light->drawLightMesh(deferredLightingShader);
            // 注意：此时光体积的模板值会被设置为1（或其他非0值），表示该区域有光照影响

            // === 第二步：渲染光照（仅标记区域） ===
            glDisable(GL_DEPTH_TEST);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);
            glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);

            // 再次绘制光体积（实际渲染光照）
            light->drawLightMesh(deferredLightingShader);
            glCullFace(GL_BACK);
            glDisable(GL_BLEND);
        }
        else
        {
            glDisable(GL_STENCIL_TEST); // 关闭模板测试
            glDisable(GL_CULL_FACE);    // 关闭面剔除
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);
            light->drawLightMesh(deferredLightingShader);
        }
    }

    // 恢复状态
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

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
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    skyboxShader->Use();
    // 设置环境贴图
    skyboxShader->SetInt("skybox", 0);
    // 设置视图矩阵(移除平移部分)
    glm::mat4 view = glm::mat4(glm::mat3(mainCamera->GetViewMatrix()));
    skyboxShader->SetMat4("view", view);
    skyboxShader->SetMat4("projection", mainCamera->GetProjectionMatrix(static_cast<float>(width) / height));
    if (gammaCorrection)
    {
        skyboxShader->SetBool("gammaEnabled", true);
    }
    else
    {
        skyboxShader->SetBool("gammaEnabled", false);
    }

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap->GetID());
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void Renderer::RenderLights()
{
    if (msaaEnabled)
    {
        hdrBufferMS->Bind();
    }
    else
    {
        hdrBuffer->Bind();
    }
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    lightsShader->Use();
    lightsShader->SetMat4("view", mainCamera->GetViewMatrix());
    lightsShader->SetMat4("projection", mainCamera->GetProjectionMatrix(width * 1.0f / height));

    for (const std::shared_ptr<Light> &light : GetLights())
    {
        std::shared_ptr<Mesh> lightMesh;
        glm::vec3 scale(0.2f);
        glm::vec3 rotation(0.0f);

        switch (light->getType())
        {
        case 0:
            lightMesh = Geometry::CreateSphere();
            lightMesh->SetTransform(light->getPosition(), rotation, scale);
            break;
        case 2: {
            auto spotLight = std::dynamic_pointer_cast<SpotLight>(light);
            if (spotLight)
            {
                // 计算圆锥的高度和半径（基于外切角）
                const float coneHeight = 1.0f; // 固定高度，美观的比例
                const float coneRadius = coneHeight * glm::tan(glm::acos(spotLight->outerCutOff));

                // 创建圆台（顶部半径为0即为圆锥）
                lightMesh = Geometry::CreateFrustum(0.0f, coneRadius, coneHeight / 2, 16);

                // 计算旋转使圆锥指向正确方向
                glm::vec3 direction = glm::normalize(spotLight->direction);
                glm::vec3 up(0.0f, 1.0f, 0.0f);

                if (glm::length(direction) > 0.001f)
                {
                    direction = glm::normalize(-direction);

                    // 计算旋转轴和角度
                    glm::vec3 axis = glm::cross(up, direction);
                    float angle = glm::acos(glm::dot(up, direction));

                    rotation = glm::degrees(axis * angle);
                }

                // 调整位置使圆锥顶点在光源位置
                // 圆锥顶点在光源位置，底部沿方向延伸
                glm::vec3 position = spotLight->position;

                // 设置变换并绘制
                lightMesh->SetTransform(position, rotation, glm::vec3(1.0f));

                // 恢复原始颜色
                lightsShader->SetVec3("lightColor", light->getLightColor());
            }
            break;
        }
        }

        if (lightMesh)
        {
            lightsShader->SetVec3("lightColor", light->getLightColor());
            lightMesh->Draw(*lightsShader);
        }
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void Renderer::RenderQuad()
{
    static GLuint quadVAO = 0, quadVBO = 0;
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::CreatePrimitive(Geometry::Type type, const glm::vec3 &position, const glm::vec3 &scale,
                               const glm::vec3 &rotation, const Material &material)
{
    Geometry::Primitive primitive;
    primitive.type = type;
    primitive.position = position;
    primitive.scale = scale;
    primitive.rotation = rotation;

    switch (type)
    {
    case Geometry::SPHERE:
        primitive.mesh = Geometry::CreateSphere();
        primitive.params = {
            .sphere = {1.0f, 16} // 默认半径和分段数
        };
        break;
    case Geometry::CUBE:
        primitive.mesh = Geometry::CreateCube();
        primitive.params = {
            .cube = {1.0f, 1.0f, 1.0f} // 默认宽度、高度和深度
        };
        break;
    case Geometry::CYLINDER:
        primitive.mesh = Geometry::CreateCylinder();
        primitive.params = {
            .cylinder = {0.5f, 1.0f, 32} // 默认半径、高度和分段数
        };
        break;
    case Geometry::CONE:
        primitive.mesh = Geometry::CreateCone();
        primitive.params = {
            .cone = {0.5f, 1.0f, 32} // 默认半径、高度和分段数
        };
        break;
    case Geometry::PRISM:
        primitive.mesh = Geometry::CreatePrism();
        primitive.params = {
            .prism = {6, 0.5f, 1.0f} // 默认六边形，半径和高度
        };
        break;
    case Geometry::PYRAMID:
        primitive.mesh = Geometry::CreatePyramid();
        primitive.params = {
            .pyramid = {4, 0.5f, 1.0f} // 默认四边形金字塔，半径和高度
        };
        break;
    case Geometry::TORUS:
        primitive.mesh = Geometry::CreateTorus();
        primitive.params = {
            .torus = {1.0f, 0.3f, 48, 12} // 默认大半径、小半径、环段数和管段数
        };
        break;
    case Geometry::ELLIPSOID:
        primitive.mesh = Geometry::CreateEllipsoid();
        primitive.params = {
            .ellipsoid = {1.0f, 0.8f, 1.2f, 32} // 默认半径和分段数
        };
        break;
    case Geometry::FRUSTUM:
        primitive.mesh = Geometry::CreateFrustum();
        primitive.params = {
            .frustum = {0.5f, 0.5f, 1.0f, 32} // 默认上底半径、下底半径、高度和分段数
        };
        break;
    case Geometry::ARROW:
        primitive.mesh = Geometry::CreateArrow(1.0f, 0.2f);
        primitive.params = {
            .arrow = {1.0f, 0.2f} // 默认长度和箭头头部大小
        };
        break;
    default:
        std::cerr << "Unsupported geometry type!" << std::endl;
        return;
    }

    primitive.mesh->SetTransform(position, rotation, scale);
    primitive.mesh->SetMaterial(std::make_shared<Material>(material));

    primitives.push_back(primitive);
}

void Renderer::LoadShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath)
{
    auto shader = std::make_shared<Shader>(vertexPath, fragmentPath);
    shaders[name] = shader;
}

std::shared_ptr<Shader> Renderer::GetShader(const std::string &name) const
{
    auto it = shaders.find(name);
    if (it != shaders.end())
    {
        return it->second;
    }
    return nullptr;
}

void Renderer::UseShader(const std::string &name)
{
    auto shader = GetShader(name);
    if (shader)
    {
        shader->Use();
    }
    else
    {
        std::cerr << "Shader not found: " << name << std::endl;
    }
}

void Renderer::UseShader(const std::shared_ptr<Shader> &shader)
{
    if (shader)
    {
        shader->Use();
    }
    else
    {
        std::cerr << "Shader is null!" << std::endl;
    }
}
void Renderer::SetGlobalUniforms(const Camera &camera)
{
    forwardShader->SetMat4("view", camera.GetViewMatrix());
    forwardShader->SetMat4("projection", camera.GetProjectionMatrix(static_cast<float>(width) / height));
    forwardShader->SetVec3("viewPos", camera.Position);
}

void Renderer::RenderPostProcessing()
{
    if (msaaEnabled)
    {
        hdrBuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        postShaderMS->Use();
        postShaderMS->SetInt("uSamples", 4);
        hdrBufferMS->BindTexture(0, 0); // 绑定多重采样的 HDR 纹理
        RenderQuad();
    }

    // 1. SSAO Pass
    if (ssaoEnabled && currentMode == DEFERRED)
    {
        ssaoBuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoShader->Use();
        // 绑定 G-Buffer 纹理、投影矩阵、噪声贴图、kernel
        RenderQuad();
    }

    // 2. Bloom Prefilter
    if (bloomEnabled)
    {
        bloomPrefilterBuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT);
        bloomPreShader->Use();
        bloomPreShader->SetFloat("threshold", 1.0f);
        hdrBuffer->BindTexture(0, 0); // scene
        RenderQuad();

        // 3. Bloom Blur (Ping-Pong)
        bool horizontal = true;
        for (int i = 0; i < 10; ++i)
        {
            bloomBlurBuffers[horizontal]->Bind();
            bloomBlurShader->Use();
            bloomBlurShader->SetBool("horizontal", horizontal);
            (i == 0 ? bloomPrefilterBuffer : bloomBlurBuffers[!horizontal])->BindTexture(0, 0);
            RenderQuad();
            horizontal = !horizontal;
        }
    }

    // 4. Final Compose
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    postProcessShader->Use();
    postProcessShader->SetBool("hdrEnabled", hdrEnabled);
    postProcessShader->SetBool("bloomEnabled", bloomEnabled);
    postProcessShader->SetBool("ssaoEnabled", ssaoEnabled && currentMode == DEFERRED);
    postProcessShader->SetBool("gammaEnabled", gammaCorrection);
    postProcessShader->SetFloat("exposure", 1.0f);

    // 绑定 HDR、Bloom、SSAO 纹理
    postProcessShader->SetInt("scene", 0);
    postProcessShader->SetInt("bloom", 1);
    postProcessShader->SetInt("ssao", 2);
    hdrBuffer->BindTexture(0, 0);
    if (bloomEnabled)
        bloomBlurBuffers[false]->BindTexture(0, 1);
    if (ssaoEnabled && currentMode == DEFERRED)
        ssaoBuffer->BindTexture(0, 2);

    RenderQuad();
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
    if (std::dynamic_pointer_cast<PointLight>(light))
    {
        pointLights.push_back(std::dynamic_pointer_cast<PointLight>(light));
    }
    else if (std::dynamic_pointer_cast<DirectionalLight>(light))
    {
        directionalLights.push_back(std::dynamic_pointer_cast<DirectionalLight>(light));
    }
    else if (std::dynamic_pointer_cast<SpotLight>(light))
    {
        spotLights.push_back(std::dynamic_pointer_cast<SpotLight>(light));
    }
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
    if (width == newWidth && height == newHeight)
        return; // 避免重复

    // 1. 解绑所有当前绑定的 FBO，防止驱动还在用
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, newWidth, newHeight);

    width = newWidth;
    height = newHeight;

    // 更新帧缓冲和着色器
    gBuffer->Resize(newWidth, newHeight);
    hdrBuffer->Resize(newWidth, newHeight);
    hdrBufferMS->Resize(newWidth, newHeight);
    bloomBlurBuffers[0]->Resize(newWidth, newHeight);
    bloomBlurBuffers[1]->Resize(newWidth, newHeight);
    bloomPrefilterBuffer->Resize(newWidth, newHeight);
    ssaoBuffer->Resize(newWidth, newHeight);
    shadowBuffer->Resize(2048, 2048); // 阴影缓冲大小固定
}