#pragma once

#include "core/Renderer.hpp"
#include "ui/EditorUI.hpp"
#include <GLFW/glfw3.h>
#include <memory>


class Application
{
  public:
    Application();
    ~Application();

    void Run();

  private:
    void Initialize();
    void MainLoop();
    void Shutdown();

    void ProcessInput(float deltaTime);
    void Update(float deltaTime);
    void Render();

    GLFWwindow *window;
    int width = 1280;
    int height = 720;

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<EditorUI> editorUI;

    bool vsyncEnabled = true;
    bool wireframeMode = false;
};