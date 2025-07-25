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
        if (io.WantCaptureMouse || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
        {
            // 如果不是鼠标右键按下，或者不需要鼠标控制相机，则不处理鼠标移动
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