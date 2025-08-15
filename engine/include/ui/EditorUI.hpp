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
#include <unordered_map>

class Renderer;

// 抗锯齿类型枚举
enum class AntiAliasingType
{
    NONE = 0,
    MSAA_2X,
    MSAA_4X,
    MSAA_8X,
    MSAA_16X,
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
    bool isPreloaded = false;       // 是否已预加载
    std::shared_ptr<Texture> previewTexture; // 预览纹理
    std::filesystem::file_time_type lastWriteTime{};
    // 缓存的资源数据
    union {
        struct {
            int width, height, channels;
        } textureData;
        struct {
            int vertexCount, faceCount;
        } modelData;
    } cachedData;
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
    void ShowAssetPreviewWindow(); // 资源预览窗口

    // 对话框
    void ShowPrimitiveSelectionDialog();
    void ShowLightSelectionDialog();
    void ShowModelCreationDialog();
    void ShowMaterialApplicationDialog(); // 材质应用对话框
    
    // 视口鼠标控制
    bool IsMouseInViewport() const { return isMouseInViewport; }
    
    // 检视器GUI
    void OnLightInspectorGUI(Light &light);

private:
    // 界面组件
    void ShowMaterialEditor(Material &material);
    void TextureSelector(const std::string &label, std::shared_ptr<Texture> &texture,
                         const std::string &idSuffix = "");
    void ApplyMaterialToObject(); // 应用材质到对象的具体实现
    void ApplyAssetToMesh(std::shared_ptr<Mesh> mesh); // 应用资源到mesh
    void ApplyAssetToPrimitive(Geometry::Primitive& primitive); // 应用资源到几何体
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
    
    // 资源预加载和管理
    void PreloadAsset(AssetItem &item);
    void UnloadAsset(AssetItem &item);
    bool LoadAssetData(AssetItem &item);
    void CreateAssetPreview(AssetItem &item);
    void ShowAssetDetailWindow(const AssetItem &item);
    void ApplyAssetToSelected(const AssetItem &item);
    void ShowTexturePreview(const AssetItem &item);
    void ShowModelPreview(const AssetItem &item);
    
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
    bool showConsole = true;
    bool showAssetPreview = false; // 资源预览窗口

    // 选择状态
    int selectedObjectIndex = -1;
    int selectedAssetIndex = -1;
    
    // 资源管理
    std::vector<AssetItem> assetItems;
    std::string currentAssetPath = "resources"; // 默认资源路径
    std::vector<std::string> assetPathHistory;
    
    // 资源预览和管理
    AssetItem* selectedAsset = nullptr; // 当前选中的资源
    std::unordered_map<std::string, std::shared_ptr<void>> preloadedAssets; // 预加载资源缓存
    bool enableAssetPreloading = true; // 是否启用资源预加载
    float previewWindowSize = 256.0f; // 预览窗口大小
    
    // 渲染设置
    AntiAliasingType currentAAType = AntiAliasingType::NONE;
    int msaaSamples = 4;
    
    // 天空盒导入状态
    bool showSkyboxImportDialog = false;
    std::vector<std::string> skyboxFaces = {"", "", "", "", "", ""}; // 6个面的路径：右、左、上、下、前、后
    
    // 场景层级编辑状态
    bool isRenamingObject = false;
    int renamingObjectIndex = -1;
    char renameBuffer[256] = "";
    
    // 材质应用到选中对象的状态
    bool showMaterialApplicationDialog = false;
    AssetItem pendingAssetToApply;  // 待应用的资源
    int selectedMeshIndex = -1;     // 选中的mesh索引
    int selectedMaterialProperty = 0; // 选中的材质属性（0=整个材质，1=漫反射，2=粗糙度等）
    
    // 视口鼠标控制状态
    bool isMouseInViewport = false;  // 鼠标是否在视口内
    ImVec2 viewportMin, viewportMax; // 视口区域边界
    
    // 复制粘贴状态
    struct CopiedObject {
        enum Type { MODEL, PRIMITIVE, LIGHT } type;
        int originalIndex;
        bool isValid = false;
        
        // 几何体数据
        struct PrimitiveData {
            glm::vec3 position{0.0f}, rotation{0.0f}, scale{1.0f};
            // 材质属性
            glm::vec3 diffuse{0.8f}, specular{0.5f}, albedo{0.8f};
            float shininess = 32.0f, metallic = 0.0f, roughness = 0.5f, ao = 1.0f;
            MaterialType materialType = BLINN_PHONG;
            bool useDiffuseMap = false, useSpecularMap = false, useNormalMap = false;
            bool useAlbedoMap = false, useMetallicMap = false, useRoughnessMap = false, useAOMap = false;
            // 纹理路径
            std::string diffuseMapPath, specularMapPath, normalMapPath;
            std::string albedoMapPath, metallicMapPath, roughnessMapPath, aoMapPath;
        } primitiveData;
        
        // 光源数据
        struct LightData {
            glm::vec3 position{0.0f}, direction{0.0f, -1.0f, 0.0f};
            glm::vec3 ambient{1.0f}, diffuse{1.0f}, specular{1.0f};
            float constant = 1.0f, linear = 0.09f, quadratic = 0.032f;
            float cutOff = 12.5f, outerCutOff = 17.5f;
            int lightType = 0; // 0=点光源, 1=方向光, 2=聚光灯
        } lightData;
    };
    CopiedObject copiedObject;
    
    // UI状态
    bool isDockingSetup = false;
    float leftPanelWidth = 300.0f;
    float rightPanelWidth = 350.0f;
    float bottomPanelHeight = 200.0f;
    
    // 光源名称映射（用于重命名功能）
    std::unordered_map<size_t, std::string> lightNames;
    
    // 控制台日志
    // 注意：控制台日志现在由Application管理
    
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