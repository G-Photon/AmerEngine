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

// 抗锯齿类型枚举
enum class AntiAliasingType
{
    NONE = 0,
    MSAA_2X,
    MSAA_4X,
    MSAA_8X,
    FXAA
};

// 资源类型枚举
enum class AssetType
{
    TEXTURE,
    MODEL,
    MATERIAL,
    SHADER,
    AUDIO,
    UNKNOWN
};

// 资源项结构
struct AssetItem
{
    std::filesystem::path path;
    AssetType type;
    std::string name;
    std::shared_ptr<void> resource; // 通用资源指针
    bool isLoaded = false;
};

class EditorUI
{
public:
    EditorUI(GLFWwindow *window, Renderer *renderer);
    ~EditorUI();

    void Initialize();
    void Update(float deltaTime);
    void BeginFrame();
    void Render();
    void EndFrame();

    // 主界面面板
    void ShowMainMenuBar();
    void ShowDockSpace();
    void ShowSceneHierarchy();
    void ShowInspector();
    void ShowAssetsPanel();
    void ShowViewport();
    void ShowRendererSettings();
    void ShowMaterialEditor();
    void ShowConsole(); // 控制台面板

    // 对话框
    void ShowPrimitiveSelectionDialog();
    void ShowLightSelectionDialog();
    void ShowModelCreationDialog();
    
    // 检视器GUI
    void OnLightInspectorGUI(Light &light);

private:
    // 界面组件
    void ShowMaterialEditor(Material &material);
    void TextureSelector(const std::string &label, std::shared_ptr<Texture> &texture,
                         const std::string &idSuffix = "");
    void ShowAntiAliasingSettings();
    void ShowPostProcessSettings();
    void ShowShadowSettings();
    void ShowLightingSettings();
    
    // 资源管理
    void RefreshAssetList();
    void LoadAssetPreview(AssetItem &item);
    void ShowAssetContextMenu(AssetItem &item);
    AssetType DetermineAssetType(const std::filesystem::path &path);
    void CreateDefaultLayout();
    
    // 样式设置
    void SetupModernStyle();
    void SetupColors();
    
    // 实用工具
    void DrawSeparator();
    void DrawTooltip(const char* text);
    bool DrawButton(const char* label, const ImVec2& size = ImVec2(0, 0));
    void DrawTextCentered(const char* text);

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
    bool showAssetsPanel = true;
    bool showViewport = true;
    bool showRendererSettings = true;
    bool showMaterialEditor = true;
    bool showConsole = false;

    // 选择状态
    int selectedObjectIndex = -1;
    int selectedAssetIndex = -1;
    
    // 资源管理
    std::vector<AssetItem> assetItems;
    std::string currentAssetPath = "resources"; // 默认资源路径
    std::vector<std::string> assetPathHistory;
    
    // 渲染设置
    AntiAliasingType currentAAType = AntiAliasingType::NONE;
    int msaaSamples = 4;
    
    // UI状态
    bool isDockingSetup = false;
    float leftPanelWidth = 300.0f;
    float rightPanelWidth = 350.0f;
    float bottomPanelHeight = 200.0f;
    
    // 控制台日志
    std::vector<std::string> consoleLog;
    
    // 场景操作状态通知
    struct Notification {
        std::string message;
        float duration;
        float timer;
        bool isSuccess;
    };
    std::vector<Notification> notifications;
    
    void AddNotification(const std::string& message, bool isSuccess = true, float duration = 3.0f);
    void UpdateNotifications(float deltaTime);
    void DrawNotifications();
    
    // 样式常量
    static constexpr float BUTTON_HEIGHT = 25.0f;
    static constexpr float ITEM_SPACING = 8.0f;
    static constexpr float PANEL_PADDING = 10.0f;
};