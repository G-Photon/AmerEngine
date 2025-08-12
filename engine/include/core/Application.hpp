#pragma once

#include "core/Renderer.hpp"
#include "ui/EditorUI.hpp"
#include <GLFW/glfw3.h>
#include <memory>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <vector>
#include <string>
#include <mutex>


class Application
{
  public:
    Application();
    ~Application();

    void Run();
    
    // 静态控制台重定向管理
    static void InitializeConsoleRedirection();
    static void ShutdownConsoleRedirection();
    static void AddConsoleLog(const std::string& message);
    static std::vector<std::string> GetConsoleLogs();
    static void ClearConsoleLogs();

  private:
    void Initialize();
    void MainLoop();
    void Shutdown();

    void ProcessInput(float deltaTime);
    void Update(float deltaTime);
    void Render();
    void CreateTestScene();

    GLFWwindow *window;
    int width = 1280;
    int height = 720;

    float lastX = width / 2.0f;
    float lastY = height / 2.0f;
    bool firstMouse = true;

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<EditorUI> editorUI;

    bool vsyncEnabled = true;
    bool wireframeMode = false;
    
    // 静态控制台重定向成员
    static std::vector<std::string> consoleLog;
    static std::mutex consoleLogMutex;
    
    class ConsoleRedirector : public std::streambuf {
    public:
        ConsoleRedirector() = default;
        
    protected:
        int overflow(int c) override {
            if (c != EOF) {
                buffer_ += static_cast<char>(c);
                if (c == '\n') {
                    AddConsoleLog(buffer_);
                    buffer_.clear();
                }
            }
            return c;
        }
        
    private:
        std::string buffer_;
    };
    
    static std::unique_ptr<ConsoleRedirector> consoleRedirector;
    static std::streambuf* originalCoutBuffer;
    static std::streambuf* originalCerrBuffer;
};