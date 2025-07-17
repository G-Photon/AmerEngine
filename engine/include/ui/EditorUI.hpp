#pragma once
#include "core/Material.hpp"
#include "core/Texture.hpp"
#include "core/Renderer.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <string>

class Renderer;

class EditorUI
{
  public:
    EditorUI(GLFWwindow *window, Renderer *renderer);
    ~EditorUI();

    void Initialize();
    void BeginFrame();
    void Render();
    void EndFrame();

    void ShowMainMenuBar();
    void ShowSceneHierarchy();
    void ShowInspector();
    void ShowRendererSettings();
    void ShowMaterialEditor();
    void ShowPrimitiveSelectionDialog();
    void ShowLightSelectionDialog();
    void ShowModelCreationDialog();
  private:
    void ShowMaterialEditor(Material &material);
    bool TextureSelector(const std::string &label, std::shared_ptr<Texture> &texture, const std::string &idSuffix = "");

    GLFWwindow *window;
    Renderer *renderer;

    bool showDemoWindow = false;
    bool showSceneHierarchy = true;
    bool showInspector = true;
    bool showRendererSettings = true;
    bool showMaterialEditor = true;

    int selectedObjectIndex = -1;
};