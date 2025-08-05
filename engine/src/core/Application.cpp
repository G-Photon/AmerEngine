#include "core/Application.hpp"
#include "core/Light.hpp"
#include "core/Material.hpp"
#include "GLFW/glfw3.h"
#include "utils/Logger.hpp"

Application::Application()
{
    Initialize();
}

Application::~Application()
{
    Shutdown();
}

void Application::Initialize()
{
    // 初始化GLFW
    if (!glfwInit())
    {
        Logger::Error("Failed to initialize GLFW");
        return;
    }

    // 配置GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

    // 创建窗口
    window = glfwCreateWindow(width, height, "AmerEngine", nullptr, nullptr);
    if (!window)
    {
        Logger::Error("Failed to create GLFW window");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(vsyncEnabled ? 1 : 0);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        Logger::Error("Failed to initialize GLAD");
        return;
    }

    // 初始化渲染器
    renderer = std::make_unique<Renderer>(width, height);
    renderer->Initialize();

    // 创建测试场景
    CreateTestScene();

    // 初始化UI
    editorUI = std::make_unique<EditorUI>(window, renderer.get());
    editorUI->Initialize();

    // 设置回调
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        if (width == 0 || height == 0)
            return; // 最小化窗口时可能触发
        auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->renderer->Resize(width, height);
        glViewport(0, 0, width, height);
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
        auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        ImGuiIO &io = ImGui::GetIO();
        
        // 只有在右键按下时才进行摄像头控制，忽略ImGui的WantCaptureMouse
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
        {
            // 如果不是鼠标右键按下，则不处理鼠标移动
            app->firstMouse = true; // 重置鼠标位置
            return;
        }
        
        if (app->firstMouse)
        {
            app->lastX = static_cast<float>(xpos);
            app->lastY = static_cast<float>(ypos);
            app->firstMouse = false;
        }
        float xoffset = static_cast<float>(xpos - app->lastX);
        float yoffset = static_cast<float>(app->lastY - ypos); // 反转y轴
        app->lastX = static_cast<float>(xpos);
        app->lastY = static_cast<float>(ypos);
        app->renderer->GetCamera()->ProcessMouseMovement(xoffset, yoffset);
    });
    glfwSetScrollCallback(window, [](GLFWwindow *window, double xoffset, double yoffset) {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->renderer->GetCamera()->ProcessMouseScroll(static_cast<float>(yoffset));
    });
}

void Application::Run()
{
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        
        ProcessInput(deltaTime);
        Update(deltaTime);
        Render();

        glfwPollEvents();
    }
}

void Application::ProcessInput(float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 摄像机控制
    static bool rightMousePressed = false;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (!rightMousePressed)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            rightMousePressed = true;
        }
        renderer->GetCamera()->ProcessInput(window, deltaTime);
    }
    else if (rightMousePressed)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        rightMousePressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        // 处理R键按下事件
        renderer->GetCamera()->Reset();
    }

    // 切换线框模式
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
    {
        wireframeMode = !wireframeMode;
        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);
    }
}

void Application::Update(float deltaTime)
{
    renderer->Update(deltaTime);
    editorUI->Update(deltaTime);
}

void Application::Render()
{
    renderer->BeginFrame();
    renderer->RenderScene();
    editorUI->Render();
    renderer->EndFrame();
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Application::CreateTestScene()
{
    // 创建地面
    Material groundMaterial;
    groundMaterial.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    groundMaterial.specular = glm::vec3(0.1f, 0.1f, 0.1f);
    groundMaterial.shininess = 32.0f;
    
    renderer->CreatePrimitive(Geometry::Type::CUBE, 
                             glm::vec3(0.0f, -2.0f, 0.0f), 
                             glm::vec3(10.0f, 0.1f, 10.0f), 
                             glm::vec3(0.0f, 0.0f, 0.0f), 
                             groundMaterial);

    // 创建几个不同颜色的立方体
    Material redCubeMaterial;
    redCubeMaterial.diffuse = glm::vec3(0.8f, 0.2f, 0.2f);
    redCubeMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    redCubeMaterial.shininess = 64.0f;
    
    Material blueCubeMaterial;
    blueCubeMaterial.diffuse = glm::vec3(0.2f, 0.2f, 0.8f);
    blueCubeMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    blueCubeMaterial.shininess = 64.0f;
    
    Material greenCubeMaterial;
    greenCubeMaterial.diffuse = glm::vec3(0.2f, 0.8f, 0.2f);
    greenCubeMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    greenCubeMaterial.shininess = 64.0f;
    
    renderer->CreatePrimitive(Geometry::Type::CUBE, 
                             glm::vec3(-2.0f, 0.0f, 0.0f), 
                             glm::vec3(1.0f, 2.0f, 1.0f), 
                             glm::vec3(0.0f, 0.0f, 0.0f), 
                             redCubeMaterial);

    renderer->CreatePrimitive(Geometry::Type::CUBE, 
                             glm::vec3(2.0f, 0.0f, 0.0f), 
                             glm::vec3(1.0f, 2.0f, 1.0f), 
                             glm::vec3(0.0f, 0.0f, 0.0f), 
                             blueCubeMaterial);

    renderer->CreatePrimitive(Geometry::Type::CUBE, 
                             glm::vec3(0.0f, 0.0f, -2.0f), 
                             glm::vec3(1.0f, 2.0f, 1.0f), 
                             glm::vec3(0.0f, 0.0f, 0.0f), 
                             greenCubeMaterial);

    // 创建一个小球
    Material sphereMaterial;
    sphereMaterial.diffuse = glm::vec3(0.8f, 0.8f, 0.2f);
    sphereMaterial.specular = glm::vec3(0.8f, 0.8f, 0.8f);
    sphereMaterial.shininess = 128.0f;
    
    renderer->CreatePrimitive(Geometry::Type::SPHERE, 
                             glm::vec3(0.0f, 1.0f, 0.0f), 
                             glm::vec3(0.5f, 0.5f, 0.5f), 
                             glm::vec3(0.0f, 0.0f, 0.0f), 
                             sphereMaterial);

    // 创建定向光源并启用阴影
    auto directionalLight = std::make_shared<DirectionalLight>(
        glm::vec3(-0.5f, -1.0f, -0.5f),  // 方向
        glm::vec3(0.2f, 0.2f, 0.2f),     // 环境光
        glm::vec3(1.0f, 1.0f, 1.0f),     // 漫反射
        glm::vec3(1.0f, 1.0f, 1.0f),     // 镜面反射
        1.0f                             // 强度
    );
    directionalLight->SetShadowEnabled(true);
    directionalLight->shadowOrthoSize = 15.0f; // 调整阴影范围
    directionalLight->shadowNearPlane = 0.1f;
    directionalLight->shadowFarPlane = 50.0f;
    renderer->AddLight(directionalLight);

    // 创建一个点光源并启用阴影
    auto pointLight = std::make_shared<PointLight>(
        glm::vec3(0.0f, 3.0f, 0.0f),     // 位置
        glm::vec3(0.1f, 0.1f, 0.1f),     // 环境光
        glm::vec3(0.5f, 0.5f, 1.0f),     // 漫反射
        glm::vec3(1.0f, 1.0f, 1.0f),     // 镜面反射
        0.5f                             // 强度
    );
    pointLight->SetShadowEnabled(true);
    pointLight->shadowNearPlane = 0.1f;
    pointLight->shadowFarPlane = 25.0f;
    renderer->AddLight(pointLight);

    // 创建一个聚光灯并启用阴影
    auto spotLight = std::make_shared<SpotLight>(
        glm::vec3(2.0f, 4.0f, 2.0f),     // 位置
        glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f)),  // 方向（归一化，指向场景中心）
        glm::vec3(0.05f, 0.05f, 0.05f),  // 环境光
        glm::vec3(1.0f, 0.8f, 0.6f),     // 漫反射（暖色调）
        glm::vec3(1.0f, 1.0f, 1.0f),     // 镜面反射
        0.8f,                            // 强度
        25.0f,                           // 内切角（度）
        35.0f                            // 外切角（度）
    );
    spotLight->SetShadowEnabled(true);
    spotLight->shadowNearPlane = 0.1f;
    spotLight->shadowFarPlane = 15.0f;
    renderer->AddLight(spotLight);

    // 启用阴影
    renderer->SetShadow(true);
}

void Application::Shutdown()
{
    editorUI.reset();
    renderer.reset();
    glfwDestroyWindow(window);
    glfwTerminate();
}