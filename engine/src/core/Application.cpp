#include "core/Application.hpp"
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
    window = glfwCreateWindow(width, height, "MyEngine", nullptr, nullptr);
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

    // 初始化UI
    editorUI = std::make_unique<EditorUI>(window, renderer.get());
    editorUI->Initialize();

    // 设置回调
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->renderer->Resize(width, height);
        glViewport(0, 0, width, height);
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

void Application::Shutdown()
{
    editorUI.reset();
    renderer.reset();
    glfwDestroyWindow(window);
    glfwTerminate();
}