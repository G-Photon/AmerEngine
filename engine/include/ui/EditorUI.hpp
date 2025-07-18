#pragma once
#include "core/Light.hpp"
#include "core/Material.hpp"
#include "core/Texture.hpp"
#include "core/Renderer.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <codecvt>
#include <locale>

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
    void ShowAssetsPanel(); // 新增资源面板
    void ShowViewport();    // 新增视口面板
    void ShowRendererSettings();
    void ShowMaterialEditor();
    void ShowPrimitiveSelectionDialog();
    void ShowLightSelectionDialog();
    void ShowModelCreationDialog();
    void OnLightInspectorGUI(Light &light);

private:
    void ShowMaterialEditor(Material &material);
    void TextureSelector(const std::string &label, std::shared_ptr<Texture> &texture,
                         const std::string &idSuffix = "");
    
    void RefreshAssetList(); // 刷新资源列表
    void CreateDefaultLayout(); // 创建默认布局

    static std::string ConvertToUTF8(const std::wstring &wstr)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(wstr);
    }

    GLFWwindow *window;
    Renderer *renderer;

    // 窗口显示状态
    bool showDemoWindow = false;
    bool showSceneHierarchy = true;
    bool showInspector = true;
    bool showAssetsPanel = true; // 新增资源面板状态
    bool showViewport = true;    // 新增视口面板状态
    bool showRendererSettings = true;
    bool showMaterialEditor = true;

    int selectedObjectIndex = -1;
    
    // 资源管理相关
    std::vector<std::filesystem::path> assetFiles;
    std::string currentAssetPath = "assets"; // 默认资源路径
};