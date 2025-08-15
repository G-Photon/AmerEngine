#include "ui/EditorUI.hpp"
#include "core/Application.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "utils/FileDialog.hpp"
#include "utils/FileSystem.hpp"
#include <cstdio>
#include <functional>
#include <algorithm>
#include <filesystem>

EditorUI::EditorUI(GLFWwindow *window, Renderer *renderer) : window(window), renderer(renderer)
{
}

EditorUI::~EditorUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorUI::Initialize()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    
    // 加载中文字体
    io.Fonts->AddFontFromFileTTF(FileSystem::GetPath("resources/fonts/HarmonyOS_Sans_SC_Medium.ttf").c_str(), 16.0f,
                                 NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    
    // 启用停靠和多视口
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // 设置现代化样式
    SetupModernStyle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    
    // 刷新资源列表
    RefreshAssetList();
}

void EditorUI::SetupModernStyle()
{
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    // 圆角和间距
    style.WindowRounding = 6.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 12.0f;
    
    // 边框
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    
    SetupColors();
}

void EditorUI::SetupColors()
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    
    // 主色调：深蓝灰
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.12f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.12f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.25f, 0.35f, 0.50f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.10f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.04f, 0.04f, 0.08f, 0.75f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.04f, 0.04f, 0.08f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void EditorUI::Update(float deltaTime)
{
    UpdateNotifications(deltaTime);
}

void EditorUI::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ShowDockSpace();
    ShowMainMenuBar();
    
    if (showSceneHierarchy)
        ShowSceneHierarchy();
    if (showInspector)
        ShowInspector();
    if (showAssetsPanel)
        ShowAssetsPanel();
    
    if (showAssetPreview && selectedAsset)
        ShowAssetPreviewWindow();
    if (showViewport)
        ShowViewport();
    if (showRendererSettings)
        ShowRendererSettings();
    if (showMaterialEditor)
        ShowMaterialEditor();
    if (showConsole)
        ShowConsole();
        
    // 对话框
    if (showMaterialApplicationDialog)
        ShowMaterialApplicationDialog();
        
    // 绘制通知
    DrawNotifications();
        
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // 多视口支持
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void EditorUI::ShowDockSpace()
{
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        
        // 首次设置默认布局
        if (!isDockingSetup)
        {
            CreateDefaultLayout();
            isDockingSetup = true;
        }
    }

    ImGui::End();
}

void EditorUI::CreateDefaultLayout()
{
    // 这里可以设置默认的停靠布局
    // 由于ImGui的限制，我们暂时让用户手动布局
}

void EditorUI::ShowAssetsPanel()
{
    ImGui::Begin(ConvertToUTF8(L"资源管理").c_str(), &showAssetsPanel);

    // 顶部工具栏
    ImGui::Checkbox(ConvertToUTF8(L"预加载").c_str(), &enableAssetPreloading);
    ImGui::SameLine();
    if (ImGui::Button(ConvertToUTF8(L"清除缓存").c_str()))
    {
        preloadedAssets.clear();
        for (auto& item : assetItems)
        {
            item.isPreloaded = false;
            item.previewTexture.reset();
        }
    }
    ImGui::Text(ConvertToUTF8(L"路径: %s").c_str(), currentAssetPath.c_str());

    ImGui::Separator();

    // 筛选器和排序
    static char filter[256] = "";
    ImGui::InputText(ConvertToUTF8(L"名称过滤").c_str(), filter, sizeof(filter));
    ImGui::SameLine();
    static int typeFilter = 0;
    static std::vector<std::string> typeItemsStr = {
        ConvertToUTF8(L"全部"),
        ConvertToUTF8(L"图片"),
        ConvertToUTF8(L"模型"),
        ConvertToUTF8(L"材质"),
        ConvertToUTF8(L"着色器"),
        ConvertToUTF8(L"音频"),
        ConvertToUTF8(L"其它")
    };
    std::vector<const char*> typeItems;
    for (const auto& s : typeItemsStr) typeItems.push_back(s.c_str());
    ImGui::SetNextItemWidth(100);
    ImGui::Combo("##TypeFilter", &typeFilter, typeItems.data(), (int)typeItems.size());
    ImGui::SameLine();
    static int sortMode = 0;
    static std::vector<std::string> sortItemsStr = {
        ConvertToUTF8(L"名称升序"),
        ConvertToUTF8(L"名称降序"),
        ConvertToUTF8(L"类型"),
        ConvertToUTF8(L"时间")
    };
    std::vector<const char*> sortItems;
    for (const auto& s : sortItemsStr) sortItems.push_back(s.c_str());
    ImGui::SetNextItemWidth(100);
    ImGui::Combo("##SortMode", &sortMode, sortItems.data(), (int)sortItems.size());

    ImGui::Separator();

    // 排序 assetItems
    static std::vector<AssetItem> sortedAssets;
    sortedAssets = assetItems;
    // 类型筛选
    auto typeMatch = [](AssetType type, int filter) {
        switch (filter) {
            case 0: return true;
            case 1: return type == AssetType::TEXTURE;
            case 2: return type == AssetType::MODEL;
            case 3: return type == AssetType::MATERIAL;
            case 4: return type == AssetType::SHADER;
            case 5: return type == AssetType::AUDIO;
            case 6: return type == AssetType::UNKNOWN;
            default: return true;
        }
    };
    // 名称过滤和类型筛选
    sortedAssets.erase(
        std::remove_if(sortedAssets.begin(), sortedAssets.end(), [&](const AssetItem& item) {
            if (strlen(filter) > 0 && item.name.find(filter) == std::string::npos)
                return true;
            if (!typeMatch(item.type, typeFilter))
                return true;
            return false;
        }),
        sortedAssets.end()
    );
    // 排序
    switch (sortMode) {
        case 0:
            std::sort(sortedAssets.begin(), sortedAssets.end(), [](const AssetItem& a, const AssetItem& b) { return a.name < b.name; });
            break;
        case 1:
            std::sort(sortedAssets.begin(), sortedAssets.end(), [](const AssetItem& a, const AssetItem& b) { return a.name > b.name; });
            break;
        case 2:
            std::sort(sortedAssets.begin(), sortedAssets.end(), [](const AssetItem& a, const AssetItem& b) { return a.type < b.type; });
            break;
        case 3:
            std::sort(sortedAssets.begin(), sortedAssets.end(), [](const AssetItem& a, const AssetItem& b) { return a.lastWriteTime > b.lastWriteTime; });
            break;
    }

    // 资源列表
    ImGui::BeginChild("AssetList", ImVec2(0, 0), true);
    for (size_t i = 0; i < sortedAssets.size(); ++i)
    {
        const auto& asset = sortedAssets[i];
        ImGui::PushID((int)i);
        // 资源图标
        const char* icon = "[FILE]";
        switch (asset.type)
        {
            case AssetType::TEXTURE: icon = "[IMG]"; break;
            case AssetType::MODEL: icon = "[3D]"; break;
            case AssetType::MATERIAL: icon = "[MAT]"; break;
            case AssetType::SHADER: icon = "[SHD]"; break;
            case AssetType::AUDIO: icon = "[AUD]"; break;
            default: icon = "[FILE]"; break;
        }
        // 选中逻辑
        bool isSelected = (selectedAsset && asset.path == selectedAsset->path);
        if (ImGui::Selectable((std::string(icon) + " " + asset.name).c_str(), isSelected))
        {
            // 重新定位到原始 assetItems 的索引
            auto it = std::find_if(assetItems.begin(), assetItems.end(), [&](const AssetItem& item) { return item.path == asset.path; });
            if (it != assetItems.end()) {
                selectedAssetIndex = (int)std::distance(assetItems.begin(), it);
                selectedAsset = &(*it);
                if (enableAssetPreloading && !selectedAsset->isPreloaded)
                {
                    PreloadAsset(*selectedAsset);
                }
            }
        }
        // 双击打开预览窗口
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            showAssetPreview = true;
            auto it = std::find_if(assetItems.begin(), assetItems.end(), [&](const AssetItem& item) { return item.path == asset.path; });
            if (it != assetItems.end()) {
                selectedAsset = &(*it);
                if (enableAssetPreloading && !selectedAsset->isPreloaded)
                {
                    PreloadAsset(*selectedAsset);
                }
            }
        }
        // 右键菜单
        if (ImGui::BeginPopupContextItem())
        {
            auto it = std::find_if(assetItems.begin(), assetItems.end(), [&](const AssetItem& item) { return item.path == asset.path; });
            if (it != assetItems.end())
                ShowAssetContextMenu(*const_cast<AssetItem*>(&(*it)));
            ImGui::EndPopup();
        }
        // 拖拽支持
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("ASSET_ITEM", &i, sizeof(size_t));
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::End();
}

void EditorUI::ShowViewport()
{
    ImGui::Begin(ConvertToUTF8(L"视口").c_str(), &showViewport);
    // 获取可用空间
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    // 检查视口尺寸是否变化
    static ImVec2 prevSize = viewportSize;
    if (prevSize.x != viewportSize.x || prevSize.y != viewportSize.y)
    {
        if (viewportSize.x > 0 && viewportSize.y > 0)
        {
            renderer->Resize(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
            prevSize = viewportSize;
        }
    }

    // 显示渲染纹理
    GLuint textureID = renderer->GetViewportTexture();
    if (textureID != 0)
    {
        // 注意：OpenGL纹理坐标原点在左下，ImGui在左上，所以需要翻转Y轴
        ImGui::Image((void *)(intptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    else
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No viewport texture available");
    }

    ImGui::End();
}

void EditorUI::ShowConsole()
{
    ImGui::Begin(ConvertToUTF8(L"控制台").c_str(), &showConsole);
    
    // 控制台工具栏
    if (ImGui::Button(ConvertToUTF8(L"清除").c_str()))
    {
        Application::ClearConsoleLogs();
    }
    ImGui::SameLine();
    
    static bool autoscroll = true;
    ImGui::Checkbox(ConvertToUTF8(L"自动滚动").c_str(), &autoscroll);
    
    ImGui::Separator();
    
    // 日志区域
    ImGui::BeginChild("ConsoleLog", ImVec2(0, 0), true);
    
    // 获取控制台日志
    auto logCopy = Application::GetConsoleLogs();
    
    for (const auto& log : logCopy)
    {
        ImGui::TextWrapped("%s", log.c_str());
    }
    
    if (autoscroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    
    ImGui::EndChild();
    ImGui::End();
}

void EditorUI::RefreshAssetList()
{
    assetItems.clear();
    
    try {
        if (std::filesystem::exists(currentAssetPath))
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(currentAssetPath))
            {
                if (entry.is_regular_file())
                {
                    AssetItem item;
                    item.path = entry.path();
                    item.name = entry.path().filename().string();
                    item.type = DetermineAssetType(entry.path());
                    item.isLoaded = false;
                    std::error_code ec;
                    item.lastWriteTime = std::filesystem::last_write_time(entry.path(), ec);
                    assetItems.push_back(item);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        Application::AddConsoleLog(ConvertToUTF8(L"刷新资源列表失败: ") + e.what());
    }
}

AssetType EditorUI::DetermineAssetType(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
        extension == ".tga" || extension == ".bmp" || extension == ".hdr")
    {
        return AssetType::TEXTURE;
    }
    else if (extension == ".obj" || extension == ".fbx" || extension == ".gltf" || extension == ".glb" ||
             extension == ".dae" || extension == ".3ds" ||
             extension == ".blend" || extension == ".pmx")
    {
        return AssetType::MODEL;
    }
    else if (extension == ".vert" || extension == ".frag" || extension == ".geom" || 
             extension == ".comp" || extension == ".glsl")
    {
        return AssetType::SHADER;
    }
    else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg")
    {
        return AssetType::AUDIO;
    }
    else if (extension == ".mat")
    {
        return AssetType::MATERIAL;
    }
    
    return AssetType::UNKNOWN;
}

void EditorUI::ShowAssetContextMenu(AssetItem& item)
{
    if (ImGui::MenuItem(ConvertToUTF8(L"打开").c_str()))
    {
        // 根据资源类型执行相应操作
        switch (item.type)
        {
            case AssetType::MODEL:
                renderer->LoadModel(item.path.string());
                break;
            case AssetType::TEXTURE:
                // 加载纹理
                break;
            default:
                break;
        }
    }
    
    ImGui::Separator();
    
    // 预加载选项
    if (!item.isPreloaded)
    {
        if (ImGui::MenuItem(ConvertToUTF8(L"预加载").c_str()))
        {
            PreloadAsset(item);
        }
    }
    else
    {
        if (ImGui::MenuItem(ConvertToUTF8(L"卸载").c_str()))
        {
            UnloadAsset(item);
        }
    }
    
    // 预览选项
    if (ImGui::MenuItem(ConvertToUTF8(L"预览").c_str()))
    {
        selectedAsset = &item;
        showAssetPreview = true;
        if (!item.isPreloaded)
        {
            PreloadAsset(item);
        }
    }
    
    ImGui::Separator();
    
    // 应用到选中对象
    if (ImGui::MenuItem(ConvertToUTF8(L"应用到选中对象").c_str()))
    {
        ApplyAssetToSelected(item);
    }
    
    if (ImGui::MenuItem(ConvertToUTF8(L"重命名").c_str()))
    {
        // 重命名逻辑
        // 这里可以弹出一个输入框让用户输入新名称
    }
    
    if (ImGui::MenuItem(ConvertToUTF8(L"删除").c_str()))
    {
        // 删除文件逻辑
    }
    
    ImGui::Separator();
    
    if (ImGui::MenuItem(ConvertToUTF8(L"在文件夹中显示").c_str()))
    {
        // 在资源管理器中显示
    }
}

void EditorUI::ShowMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(ConvertToUTF8(L"文件").c_str()))
        {
            if (ImGui::MenuItem(ConvertToUTF8(L"新建场景").c_str(), "Ctrl+N")) {
                renderer->NewScene();
                AddNotification(ConvertToUTF8(L"新建场景成功"), true);
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"打开场景").c_str(), "Ctrl+O")) {
                std::string path = FileDialog::OpenFile("Open Scene", "JSON Files\0*.json\0All Files\0*.*\0");
                if (!path.empty()) {
                    try {
                        renderer->LoadScene(path);
                        AddNotification(ConvertToUTF8(L"场景加载成功: ") + std::filesystem::path(path).filename().string(), true);
                    } catch (const std::exception& e) {
                        AddNotification(ConvertToUTF8(L"场景加载失败: ") + e.what(), false);
                    }
                }
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"保存场景").c_str(), "Ctrl+S"))
            {
                //默认保存为json
                std::string path = FileDialog::SaveFile("Save Scene", "JSON Files\0*.json\0All Files\0*.*\0");
                if (!path.empty()) {
                    try {
                        renderer->SaveScene(path);
                        AddNotification(ConvertToUTF8(L"场景保存成功: ") + std::filesystem::path(path).filename().string(), true);
                    } catch (const std::exception& e) {
                        AddNotification(ConvertToUTF8(L"场景保存失败: ") + e.what(), false);
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem(ConvertToUTF8(L"退出").c_str(), "Alt+F4")) {
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ConvertToUTF8(L"视图").c_str()))
        {
            ImGui::MenuItem(ConvertToUTF8(L"场景层级").c_str(), NULL, &showSceneHierarchy);
            ImGui::MenuItem(ConvertToUTF8(L"检视器").c_str(), NULL, &showInspector);
            ImGui::MenuItem(ConvertToUTF8(L"资源管理").c_str(), NULL, &showAssetsPanel);
            ImGui::MenuItem(ConvertToUTF8(L"资源预览").c_str(), NULL, &showAssetPreview);
            ImGui::MenuItem(ConvertToUTF8(L"视口").c_str(), NULL, &showViewport);
            ImGui::MenuItem(ConvertToUTF8(L"渲染器设置").c_str(), NULL, &showRendererSettings);
            ImGui::MenuItem(ConvertToUTF8(L"材质编辑器").c_str(), NULL, &showMaterialEditor);
            ImGui::MenuItem(ConvertToUTF8(L"控制台").c_str(), NULL, &showConsole);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ConvertToUTF8(L"创建").c_str()))
        {
            if (ImGui::BeginMenu(ConvertToUTF8(L"3D对象").c_str()))
            {
                if (ImGui::MenuItem(ConvertToUTF8(L"立方体").c_str()))
                {
                    renderer->CreatePrimitive(Geometry::CUBE, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                              Material());
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"球体").c_str()))
                {
                    renderer->CreatePrimitive(Geometry::SPHERE, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                              Material());
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"圆柱体").c_str()))
                {
                    renderer->CreatePrimitive(Geometry::CYLINDER, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                              Material());
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"圆锥体").c_str()))
                {
                    renderer->CreatePrimitive(Geometry::CONE, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                              Material());
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"环面").c_str()))
                {
                    renderer->CreatePrimitive(Geometry::TORUS, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                              Material());
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"金字塔").c_str()))
                {
                    renderer->CreatePrimitive(Geometry::PYRAMID, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                              Material());
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"椭球体").c_str()))
                {
                    renderer->CreatePrimitive(Geometry::ELLIPSOID, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                              Material());
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"圆台").c_str()))
                {
                    renderer->CreatePrimitive(Geometry::CONE, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                              Material());
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"棱柱").c_str()))
                {
                    renderer->CreatePrimitive(Geometry::PRISM, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                              Material());
                }

                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu(ConvertToUTF8(L"光源").c_str()))
            {
                if (ImGui::MenuItem(ConvertToUTF8(L"方向光").c_str())) {
                    auto light = std::make_shared<DirectionalLight>();
                    renderer->AddLight(light);
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"点光源").c_str())) {
                    auto light = std::make_shared<PointLight>();
                    renderer->AddLight(light);
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"聚光灯").c_str())) {
                    auto light = std::make_shared<SpotLight>();
                    renderer->AddLight(light);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        static bool showAboutPopup = false;
        if (ImGui::BeginMenu(ConvertToUTF8(L"帮助").c_str()))
        {
            if (ImGui::MenuItem(ConvertToUTF8(L"关于").c_str()))
            {
                showAboutPopup = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();

        if (showAboutPopup)
        {
            ImGui::OpenPopup(ConvertToUTF8(L"about").c_str());
            showAboutPopup = false;
        }

        if (ImGui::BeginPopup(ConvertToUTF8(L"about").c_str(), ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("%s", ConvertToUTF8(L"关于本软件的说明...").c_str());
            ImGui::Text("%s", ConvertToUTF8(L"版本: 1.0.0").c_str());
            ImGui::Text("%s", ConvertToUTF8(L"作者: G-Photon").c_str());
            ImGui::Text("%s", ConvertToUTF8(L"感谢使用本软件！").c_str());
            if (ImGui::Button(ConvertToUTF8(L"关闭").c_str()))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void EditorUI::ShowLightSelectionDialog()
{
    // ImGui::OpenPopup("Create Light");
    if (ImGui::BeginPopup(ConvertToUTF8(L"创建光源").c_str()))
    {
        ImGui::Text("%s", ConvertToUTF8(L"光源类型：").c_str());
        static int lightType = 0;
        ImGui::RadioButton(ConvertToUTF8(L"点光源").c_str(), &lightType, 0);
        ImGui::RadioButton(ConvertToUTF8(L"方向光源").c_str(), &lightType, 1);
        ImGui::RadioButton(ConvertToUTF8(L"聚光灯").c_str(), &lightType, 2);

        static bool enableShadows = false;
        ImGui::Checkbox(ConvertToUTF8(L"启用阴影").c_str(), &enableShadows);
        
        if (ImGui::Button(ConvertToUTF8(L"创建").c_str()))
        {
            // 创建光源逻辑
            std::shared_ptr<Light> light;
            if (lightType == 0)
            {
                light = std::make_shared<PointLight>();
            }
            else if (lightType == 1)
            {
                light = std::make_shared<DirectionalLight>();
            }
            else if (lightType == 2)
            {
                light = std::make_shared<SpotLight>();
            }
            
            // 启用阴影
            if (enableShadows)
            {
                light->SetShadowEnabled(true);
            }
            
            renderer->AddLight(light);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EditorUI::ShowSceneHierarchy()
{
    ImGui::Begin(ConvertToUTF8(L"场景层级").c_str(), &showSceneHierarchy);
    
    // 顶部工具栏
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
    if (DrawButton(ConvertToUTF8(L"+").c_str(), ImVec2(25, 25)))
    {
        ImGui::OpenPopup("CreateMenu");
    }
    DrawTooltip(ConvertToUTF8(L"创建新对象").c_str());
    
    ImGui::SameLine();
    if (DrawButton(ConvertToUTF8(L"R").c_str(), ImVec2(25, 25)))
    {
        // 刷新场景
    }
    DrawTooltip(ConvertToUTF8(L"刷新场景").c_str());
    
    ImGui::SameLine();
    if (DrawButton(ConvertToUTF8(L"X").c_str(), ImVec2(25, 25)))
    {
        if (selectedObjectIndex >= 0)
        {
            renderer->DeleteObject(selectedObjectIndex);
            selectedObjectIndex = -1;
        }
    }
    DrawTooltip(ConvertToUTF8(L"删除选中对象").c_str());
    ImGui::PopStyleVar();
    
    // 创建菜单弹出窗口
    if (ImGui::BeginPopup("CreateMenu"))
    {
        if (ImGui::BeginMenu(ConvertToUTF8(L"3D对象").c_str()))
        {
            if (ImGui::MenuItem(ConvertToUTF8(L"立方体").c_str())) {
                renderer->CreatePrimitive(Geometry::CUBE, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material());
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"球体").c_str())) {
                renderer->CreatePrimitive(Geometry::SPHERE, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material());
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"圆柱体").c_str())) {
                renderer->CreatePrimitive(Geometry::CYLINDER, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material());
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"圆锥体").c_str())) {
                renderer->CreatePrimitive(Geometry::CONE, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material());
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"环面").c_str())) {
                renderer->CreatePrimitive(Geometry::TORUS, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material());
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"金字塔").c_str())) {
                renderer->CreatePrimitive(Geometry::PYRAMID, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material());
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"椭球体").c_str())) {
                renderer->CreatePrimitive(Geometry::ELLIPSOID, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material());
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"圆台").c_str())) {
                renderer->CreatePrimitive(Geometry::CONE, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material());
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"棱柱").c_str())) {
                renderer->CreatePrimitive(Geometry::PRISM, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), Material());
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu(ConvertToUTF8(L"光源").c_str()))
        {
            if (ImGui::MenuItem(ConvertToUTF8(L"方向光").c_str())) {
                auto light = std::make_shared<DirectionalLight>();
                renderer->AddLight(light);
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"点光源").c_str())) {
                auto light = std::make_shared<PointLight>();
                renderer->AddLight(light);
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"聚光灯").c_str())) {
                auto light = std::make_shared<SpotLight>();
                renderer->AddLight(light);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::MenuItem(ConvertToUTF8(L"导入模型...").c_str())) {
            std::string modelPath = FileDialog::OpenFile(
                ConvertToUTF8(L"选择模型文件").c_str(), 
                "Model Files\0*.obj;*.fbx;*.gltf;*.glb;*.pmx\0All Files\0*.*\0"
            );
            if (!modelPath.empty()) {
                renderer->LoadModel(modelPath);
            }
        }
        
        ImGui::EndPopup();
    }
    
    ImGui::Separator();
    
    // 搜索过滤器
    static char searchFilter[256] = "";
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##Search", ConvertToUTF8(L"搜索对象...").c_str(), searchFilter, sizeof(searchFilter));
    
    ImGui::Separator();
    
    // 场景对象列表
    ImGui::BeginChild("SceneObjects", ImVec2(0, 0), true);
    
    // 模型节点
    if (ImGui::TreeNodeEx(ConvertToUTF8(L"[模型] 模型").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (int i = 0; i < renderer->GetModelCount(); ++i)
        {
            auto model = renderer->GetModel(i);
            std::string name = "[3D] " + model->GetName();
            
            // 应用搜索过滤器
            if (strlen(searchFilter) > 0 && name.find(searchFilter) == std::string::npos)
                continue;
                
            ImGui::PushID(i);
            bool isSelected = (selectedObjectIndex == i);
            
            // 检查是否正在重命名这个对象
            if (isRenamingObject && renamingObjectIndex == i) {
                // 显示重命名输入框
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputText("##Rename", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                    // 应用重命名
                    model->SetName(renameBuffer);
                    isRenamingObject = false;
                    renamingObjectIndex = -1;
                }
                
                // 检查是否取消重命名
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    isRenamingObject = false;
                    renamingObjectIndex = -1;
                }
                
                // 自动聚焦输入框
                if (ImGui::IsItemActivated()) {
                    ImGui::SetKeyboardFocusHere(-1);
                }
            } else {
                // 正常显示对象名称
                if (ImGui::Selectable(name.c_str(), isSelected))
                {
                    selectedObjectIndex = i;
                }
            }
            
            // 右键菜单
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem(ConvertToUTF8(L"重命名").c_str())) {
                    isRenamingObject = true;
                    renamingObjectIndex = i;
                    strncpy_s(renameBuffer, model->GetName().c_str(), sizeof(renameBuffer) - 1);
                    renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"复制").c_str())) {
                    // 模型复制功能暂未实现
                    Application::AddConsoleLog("Model duplication not yet implemented");
                }
                ImGui::Separator();
                if (ImGui::MenuItem(ConvertToUTF8(L"删除").c_str())) {
                    renderer->DeleteObject(i);
                    selectedObjectIndex = -1;
                }
                ImGui::EndPopup();
            }
            
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    // 几何体节点
    if (ImGui::TreeNodeEx(ConvertToUTF8(L"[几何] 几何体").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto &primitives = renderer->GetPrimitives();
        for (size_t i = 0; i < primitives.size(); ++i)
        {
            std::string name;
            
            // 检查几何体是否有自定义名称
            if (primitives[i].mesh && !primitives[i].mesh->GetName().empty() && primitives[i].mesh->GetName() != "Mesh") {
                name = "[GEOM] " + primitives[i].mesh->GetName();
            } else {
                name = "[GEOM] " + ConvertToUTF8(Geometry::name[primitives[i].type]) + " " + std::to_string(i);
            }
            
            // 应用搜索过滤器
            if (strlen(searchFilter) > 0 && name.find(searchFilter) == std::string::npos)
                continue;
                
            ImGui::PushID((int)(i + renderer->GetModelCount()));
            bool isSelected = (selectedObjectIndex == (int)(i + renderer->GetModelCount()));
            
            // 检查是否正在重命名这个对象
            if (isRenamingObject && renamingObjectIndex == (int)(i + renderer->GetModelCount())) {
                // 显示重命名输入框
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputText("##Rename", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                    // 应用重命名 - 几何体重命名功能（可以在网格中设置名称）
                    if (primitives[i].mesh) {
                        primitives[i].mesh->SetName(renameBuffer);
                        Application::AddConsoleLog("Geometry renamed to: " + std::string(renameBuffer));
                    }
                    isRenamingObject = false;
                    renamingObjectIndex = -1;
                }
                
                // 检查是否取消重命名
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    isRenamingObject = false;
                    renamingObjectIndex = -1;
                }
                
                // 自动聚焦输入框
                if (ImGui::IsItemActivated()) {
                    ImGui::SetKeyboardFocusHere(-1);
                }
            } else {
                // 正常显示对象名称
                if (ImGui::Selectable(name.c_str(), isSelected))
                {
                    selectedObjectIndex = (int)(i + renderer->GetModelCount());
                }
            }
            
            // 右键菜单
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem(ConvertToUTF8(L"重命名").c_str())) {
                    isRenamingObject = true;
                    renamingObjectIndex = (int)(i + renderer->GetModelCount());
                    if (primitives[i].mesh && !primitives[i].mesh->GetName().empty() && 
                        primitives[i].mesh->GetName() != "Mesh") {
                        // 使用现有的自定义名称
                        strncpy_s(renameBuffer, primitives[i].mesh->GetName().c_str(), sizeof(renameBuffer) - 1);
                        renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                    } else {
                        // 使用默认名称格式
                        snprintf(renameBuffer, sizeof(renameBuffer), "%s %zu", 
                                ConvertToUTF8(Geometry::name[primitives[i].type]).c_str(), i);
                    }
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"复制").c_str())) {
                    // 直接复制几何体
                    auto &originalPrimitive = primitives[i];
                    
                    // 获取原始材质
                    auto originalMaterial = originalPrimitive.mesh->GetMaterial();
                    auto newMaterial = std::make_shared<Material>(originalMaterial->type);
                    
                    // 复制所有材质属性
                    newMaterial->diffuse = originalMaterial->diffuse;
                    newMaterial->specular = originalMaterial->specular;
                    newMaterial->shininess = originalMaterial->shininess;
                    newMaterial->albedo = originalMaterial->albedo;
                    newMaterial->metallic = originalMaterial->metallic;
                    newMaterial->roughness = originalMaterial->roughness;
                    newMaterial->ao = originalMaterial->ao;
                    
                    // 复制纹理使用标志
                    newMaterial->useDiffuseMap = originalMaterial->useDiffuseMap;
                    newMaterial->useSpecularMap = originalMaterial->useSpecularMap;
                    newMaterial->useNormalMap = originalMaterial->useNormalMap;
                    newMaterial->useAlbedoMap = originalMaterial->useAlbedoMap;
                    newMaterial->useMetallicMap = originalMaterial->useMetallicMap;
                    newMaterial->useRoughnessMap = originalMaterial->useRoughnessMap;
                    newMaterial->useAOMap = originalMaterial->useAOMap;
                    
                    // 复制纹理
                    if (originalMaterial->diffuseMap) {
                        newMaterial->diffuseMap = std::make_shared<Texture>(originalMaterial->diffuseMap->GetPath());
                    }
                    if (originalMaterial->specularMap) {
                        newMaterial->specularMap = std::make_shared<Texture>(originalMaterial->specularMap->GetPath());
                    }
                    if (originalMaterial->normalMap) {
                        newMaterial->normalMap = std::make_shared<Texture>(originalMaterial->normalMap->GetPath());
                    }
                    if (originalMaterial->albedoMap) {
                        newMaterial->albedoMap = std::make_shared<Texture>(originalMaterial->albedoMap->GetPath());
                    }
                    if (originalMaterial->metallicMap) {
                        newMaterial->metallicMap = std::make_shared<Texture>(originalMaterial->metallicMap->GetPath());
                    }
                    if (originalMaterial->roughnessMap) {
                        newMaterial->roughnessMap = std::make_shared<Texture>(originalMaterial->roughnessMap->GetPath());
                    }
                    if (originalMaterial->aoMap) {
                        newMaterial->aoMap = std::make_shared<Texture>(originalMaterial->aoMap->GetPath());
                    }
                    
                    // 在相同位置创建新几何体
                    renderer->CreatePrimitive(originalPrimitive.type, 
                                            originalPrimitive.position,
                                            originalPrimitive.scale,
                                            originalPrimitive.rotation, 
                                            *newMaterial);
                    
                    Application::AddConsoleLog("Primitive duplicated with all properties");
                }
                ImGui::Separator();
                if (ImGui::MenuItem(ConvertToUTF8(L"删除").c_str())) {
                    renderer->DeleteObject((int)(i + renderer->GetModelCount()));
                    selectedObjectIndex = -1;
                }
                ImGui::EndPopup();
            }
            
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    // 光源节点
    if (ImGui::TreeNodeEx(ConvertToUTF8(L"[光照] 光源").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        const auto &lights = renderer->GetLights();
        auto &primitives = renderer->GetPrimitives();
        
        for (size_t i = 0; i < lights.size(); ++i)
        {
            auto light = lights[i];
            std::string icon = "[LIGHT]";
            std::string typeName = ConvertToUTF8(L"未知光源");
            
            // 根据光源类型设置图标和名称
            switch (light->getType())
            {
                case 0: // PointLight
                    icon = "[POINT]";
                    typeName = ConvertToUTF8(L"点光源");
                    break;
                case 1: // DirectionalLight
                    icon = "[DIR]";
                    typeName = ConvertToUTF8(L"方向光");
                    break;
                case 2: // SpotLight
                    icon = "[SPOT]";
                    typeName = ConvertToUTF8(L"聚光灯");
                    break;
            }
            
            std::string name;
            
            // 检查是否有自定义名称
            if (lightNames.find(i) != lightNames.end()) {
                name = icon + " " + lightNames[i];
            } else {
                name = icon + " " + typeName + " " + std::to_string(i);
            }
            
            // 应用搜索过滤器
            if (strlen(searchFilter) > 0 && name.find(searchFilter) == std::string::npos)
                continue;
                
            ImGui::PushID((int)(i + renderer->GetModelCount() + primitives.size()));
            bool isSelected = (selectedObjectIndex == (int)(i + renderer->GetModelCount() + primitives.size()));
            
            // 检查是否正在重命名这个对象
            if (isRenamingObject && renamingObjectIndex == (int)(i + renderer->GetModelCount() + primitives.size())) {
                // 显示重命名输入框
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputText("##Rename", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                    // 应用重命名 - 保存光源自定义名称
                    lightNames[i] = renameBuffer;
                    Application::AddConsoleLog("Light renamed to: " + std::string(renameBuffer));
                    isRenamingObject = false;
                    renamingObjectIndex = -1;
                }
                
                // 检查是否取消重命名
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    isRenamingObject = false;
                    renamingObjectIndex = -1;
                }
                
                // 自动聚焦输入框
                if (ImGui::IsItemActivated()) {
                    ImGui::SetKeyboardFocusHere(-1);
                }
            } else {
                // 正常显示对象名称
                if (ImGui::Selectable(name.c_str(), isSelected))
                {
                    selectedObjectIndex = (int)(i + renderer->GetModelCount() + primitives.size());
                }
            }
            
            // 右键菜单
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem(ConvertToUTF8(L"重命名").c_str())) {
                    isRenamingObject = true;
                    renamingObjectIndex = (int)(i + renderer->GetModelCount() + primitives.size());
                    if (lightNames.find(i) != lightNames.end()) {
                        // 使用现有的自定义名称
                        strncpy_s(renameBuffer, lightNames[i].c_str(), sizeof(renameBuffer) - 1);
                        renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                    } else {
                        // 使用默认名称格式
                        snprintf(renameBuffer, sizeof(renameBuffer), "%s %zu", typeName.c_str(), i);
                    }
                }
                if (ImGui::MenuItem(ConvertToUTF8(L"复制").c_str())) {
                    // 直接复制光源
                    auto originalLight = lights[i];
                    
                    // 根据光源类型创建副本
                    if (originalLight->getType() == 0) { // 点光源
                        auto pointLight = std::dynamic_pointer_cast<PointLight>(originalLight);
                        if (pointLight) {
                            auto newLight = std::make_shared<PointLight>(
                                pointLight->position,
                                pointLight->ambient,
                                pointLight->diffuse,
                                pointLight->specular,
                                pointLight->intensity
                            );
                            newLight->constant = pointLight->constant;
                            newLight->linear = pointLight->linear;
                            newLight->quadratic = pointLight->quadratic;
                            renderer->AddLight(newLight);
                            Application::AddConsoleLog("Point light duplicated");
                        }
                    } else if (originalLight->getType() == 1) { // 方向光
                        auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(originalLight);
                        if (dirLight) {
                            auto newLight = std::make_shared<DirectionalLight>(
                                dirLight->direction,
                                dirLight->ambient,
                                dirLight->diffuse,
                                dirLight->specular,
                                dirLight->intensity
                            );
                            renderer->AddLight(newLight);
                            Application::AddConsoleLog("Directional light duplicated");
                        }
                    } else if (originalLight->getType() == 2) { // 聚光灯
                        auto spotLight = std::dynamic_pointer_cast<SpotLight>(originalLight);
                        if (spotLight) {
                            auto newLight = std::make_shared<SpotLight>(
                                spotLight->position,
                                spotLight->direction,
                                spotLight->ambient,
                                spotLight->diffuse,
                                spotLight->specular,
                                spotLight->intensity,
                                spotLight->cutOff,
                                spotLight->outerCutOff
                            );
                            newLight->constant = spotLight->constant;
                            newLight->linear = spotLight->linear;
                            newLight->quadratic = spotLight->quadratic;
                            renderer->AddLight(newLight);
                            Application::AddConsoleLog("Spot light duplicated");
                        }
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem(ConvertToUTF8(L"删除").c_str())) {
                    renderer->DeleteObject((int)(i + renderer->GetModelCount() + primitives.size()));
                    selectedObjectIndex = -1;
                }
                ImGui::EndPopup();
            }
            
            ImGui::PopID();
        }
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
    
    // 处理键盘快捷键
    if (ImGui::IsWindowFocused()) {
        // Ctrl+C 复制选中对象（直接创建副本）
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_C) && selectedObjectIndex >= 0) {
            int modelCount = renderer->GetModelCount();
            int primitiveCount = renderer->GetPrimitives().size();
            
            if (selectedObjectIndex < modelCount) {
                // 复制模型（暂未实现）
                Application::AddConsoleLog("Model duplication not yet implemented");
            } else if (selectedObjectIndex < modelCount + primitiveCount) {
                // 直接复制几何体
                auto &primitives = renderer->GetPrimitives();
                int primitiveIndex = selectedObjectIndex - modelCount;
                auto &originalPrimitive = primitives[primitiveIndex];
                
                // 获取原始材质
                auto originalMaterial = originalPrimitive.mesh->GetMaterial();
                auto newMaterial = std::make_shared<Material>(originalMaterial->type);
                
                // 复制所有材质属性
                newMaterial->diffuse = originalMaterial->diffuse;
                newMaterial->specular = originalMaterial->specular;
                newMaterial->shininess = originalMaterial->shininess;
                newMaterial->albedo = originalMaterial->albedo;
                newMaterial->metallic = originalMaterial->metallic;
                newMaterial->roughness = originalMaterial->roughness;
                newMaterial->ao = originalMaterial->ao;
                
                // 复制纹理使用标志
                newMaterial->useDiffuseMap = originalMaterial->useDiffuseMap;
                newMaterial->useSpecularMap = originalMaterial->useSpecularMap;
                newMaterial->useNormalMap = originalMaterial->useNormalMap;
                newMaterial->useAlbedoMap = originalMaterial->useAlbedoMap;
                newMaterial->useMetallicMap = originalMaterial->useMetallicMap;
                newMaterial->useRoughnessMap = originalMaterial->useRoughnessMap;
                newMaterial->useAOMap = originalMaterial->useAOMap;
                
                // 复制纹理
                if (originalMaterial->diffuseMap) {
                    newMaterial->diffuseMap = std::make_shared<Texture>(originalMaterial->diffuseMap->GetPath());
                }
                if (originalMaterial->specularMap) {
                    newMaterial->specularMap = std::make_shared<Texture>(originalMaterial->specularMap->GetPath());
                }
                if (originalMaterial->normalMap) {
                    newMaterial->normalMap = std::make_shared<Texture>(originalMaterial->normalMap->GetPath());
                }
                if (originalMaterial->albedoMap) {
                    newMaterial->albedoMap = std::make_shared<Texture>(originalMaterial->albedoMap->GetPath());
                }
                if (originalMaterial->metallicMap) {
                    newMaterial->metallicMap = std::make_shared<Texture>(originalMaterial->metallicMap->GetPath());
                }
                if (originalMaterial->roughnessMap) {
                    newMaterial->roughnessMap = std::make_shared<Texture>(originalMaterial->roughnessMap->GetPath());
                }
                if (originalMaterial->aoMap) {
                    newMaterial->aoMap = std::make_shared<Texture>(originalMaterial->aoMap->GetPath());
                }
                
                // 在相同位置创建新几何体（完全相同的变换）
                renderer->CreatePrimitive(originalPrimitive.type, 
                                        originalPrimitive.position,
                                        originalPrimitive.scale,
                                        originalPrimitive.rotation, 
                                        *newMaterial);
                
                Application::AddConsoleLog("Primitive duplicated with all properties");
            } else {
                // 直接复制光源
                auto lights = renderer->GetLights();
                int lightIndex = selectedObjectIndex - modelCount - primitiveCount;
                if (lightIndex >= 0 && lightIndex < lights.size()) {
                    auto originalLight = lights[lightIndex];
                    
                    // 根据光源类型创建副本
                    if (originalLight->getType() == 0) { // 点光源
                        auto pointLight = std::dynamic_pointer_cast<PointLight>(originalLight);
                        if (pointLight) {
                            auto newLight = std::make_shared<PointLight>(
                                pointLight->position,
                                pointLight->ambient,
                                pointLight->diffuse,
                                pointLight->specular,
                                pointLight->intensity
                            );
                            newLight->constant = pointLight->constant;
                            newLight->linear = pointLight->linear;
                            newLight->quadratic = pointLight->quadratic;
                            renderer->AddLight(newLight);
                            Application::AddConsoleLog("Point light duplicated");
                        }
                    } else if (originalLight->getType() == 1) { // 方向光
                        auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(originalLight);
                        if (dirLight) {
                            auto newLight = std::make_shared<DirectionalLight>(
                                dirLight->direction,
                                dirLight->ambient,
                                dirLight->diffuse,
                                dirLight->specular,
                                dirLight->intensity
                            );
                            renderer->AddLight(newLight);
                            Application::AddConsoleLog("Directional light duplicated");
                        }
                    } else if (originalLight->getType() == 2) { // 聚光灯
                        auto spotLight = std::dynamic_pointer_cast<SpotLight>(originalLight);
                        if (spotLight) {
                            auto newLight = std::make_shared<SpotLight>(
                                spotLight->position,
                                spotLight->direction,
                                spotLight->ambient,
                                spotLight->diffuse,
                                spotLight->specular,
                                spotLight->intensity,
                                spotLight->cutOff,
                                spotLight->outerCutOff
                            );
                            newLight->constant = spotLight->constant;
                            newLight->linear = spotLight->linear;
                            newLight->quadratic = spotLight->quadratic;
                            renderer->AddLight(newLight);
                            Application::AddConsoleLog("Spot light duplicated");
                        }
                    }
                }
            }
        }
        
        // F2 重命名选中对象
        if (ImGui::IsKeyPressed(ImGuiKey_F2) && selectedObjectIndex >= 0 && !isRenamingObject) {
            isRenamingObject = true;
            renamingObjectIndex = selectedObjectIndex;
            
            int modelCount = renderer->GetModelCount();
            int primitiveCount = renderer->GetPrimitives().size();
            
            if (selectedObjectIndex < modelCount) {
                // 重命名模型
                auto model = renderer->GetModel(selectedObjectIndex);
                strncpy_s(renameBuffer, model->GetName().c_str(), sizeof(renameBuffer) - 1);
                renameBuffer[sizeof(renameBuffer) - 1] = '\0';
            } else if (selectedObjectIndex < modelCount + primitiveCount) {
                // 重命名几何体
                auto &primitives = renderer->GetPrimitives();
                int primitiveIndex = selectedObjectIndex - modelCount;
                if (primitives[primitiveIndex].mesh && !primitives[primitiveIndex].mesh->GetName().empty() && 
                    primitives[primitiveIndex].mesh->GetName() != "Mesh") {
                    // 使用现有的自定义名称
                    strncpy_s(renameBuffer, primitives[primitiveIndex].mesh->GetName().c_str(), sizeof(renameBuffer) - 1);
                    renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                } else {
                    // 使用默认名称格式
                    snprintf(renameBuffer, sizeof(renameBuffer), "%s %d", 
                            ConvertToUTF8(Geometry::name[primitives[primitiveIndex].type]).c_str(), primitiveIndex);
                }
            } else {
                // 重命名光源
                int lightIndex = selectedObjectIndex - modelCount - primitiveCount;
                if (lightNames.find(lightIndex) != lightNames.end()) {
                    // 使用现有的自定义名称
                    strncpy_s(renameBuffer, lightNames[lightIndex].c_str(), sizeof(renameBuffer) - 1);
                    renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                } else {
                    // 使用默认名称格式
                    snprintf(renameBuffer, sizeof(renameBuffer), "Light %d", lightIndex);
                }
            }
        }
        
        // Delete 删除选中对象
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && selectedObjectIndex >= 0) {
            renderer->DeleteObject(selectedObjectIndex);
            selectedObjectIndex = -1;
            Application::AddConsoleLog("Object deleted");
        }
    }
    
    ImGui::End();
}

void EditorUI::ShowInspector()
{
    ImGui::Begin(ConvertToUTF8(L"检视器").c_str());
    if (selectedObjectIndex >= 0)
    {
        int modelCount = renderer->GetModelCount();
        int primitiveCount = renderer->GetPrimitives().size();

        if (selectedObjectIndex < modelCount)
        {
            // 显示模型属性
            auto model = renderer->GetModel(selectedObjectIndex);
            ImGui::Text("%s: %s", ConvertToUTF8(L"模型").c_str(), model->GetName().c_str());

            // 变换编辑器
            static glm::vec3 position, rotation, scale;
            position = model->GetPosition();
            rotation = model->GetRotation();
            scale = model->GetScale();

            ImGui::DragFloat3(ConvertToUTF8(L"位置").c_str(), glm::value_ptr(position), 0.1f);
            ImGui::DragFloat3(ConvertToUTF8(L"旋转").c_str(), glm::value_ptr(rotation), 1.0f);
            ImGui::DragFloat3(ConvertToUTF8(L"缩放").c_str(), glm::value_ptr(scale), 0.1f);

            model->SetTransform(position, rotation, scale);

            // 材质编辑器 可能有很多个不同的mesh，imgui需要分配不同id
            for (auto &mesh : model->GetMeshes())
            {
                ImGui::PushID(&mesh);
                ImGui::Text("%s: %s", ConvertToUTF8(L"网格").c_str(), mesh->GetName().c_str());
                if (ImGui::TreeNode(mesh->GetName().c_str()))
                {
                    ShowMaterialEditor(*mesh->GetMaterial());
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
        else if (selectedObjectIndex < modelCount + primitiveCount)
        {
            // 显示几何体属性
            auto &primitive = renderer->GetPrimitives()[selectedObjectIndex - modelCount];
            ImGui::Text("%s: %s", ConvertToUTF8(L"几何体类型").c_str(),
                        ConvertToUTF8(Geometry::name[primitive.type]).c_str());

            // 变换编辑器
            ImGui::DragFloat3(ConvertToUTF8(L"位置").c_str(), (float *)glm::value_ptr(primitive.position), 0.1f);
            ImGui::DragFloat3(ConvertToUTF8(L"旋转").c_str(), (float *)glm::value_ptr(primitive.rotation), 1.0f);
            ImGui::DragFloat3(ConvertToUTF8(L"缩放").c_str(), (float *)glm::value_ptr(primitive.scale), 0.1f);
            // 更新几何体变换
            // 根据几何体类型显示参数编辑器
            switch (primitive.type)
            {
            case Geometry::SPHERE: {
                auto &radius = primitive.params.sphere.radius;
                auto &segments = primitive.params.sphere.segments;
                if (ImGui::DragFloat(ConvertToUTF8(L"半径").c_str(), &radius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragInt(ConvertToUTF8(L"分段数").c_str(), &segments, 1, 3, 100))
                {
                    Geometry::UpdateSphere(primitive.mesh, radius, segments);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::CUBE: {
                auto &width = primitive.params.cube.width;
                auto &height = primitive.params.cube.height;
                auto &depth = primitive.params.cube.depth;
                if (ImGui::DragFloat(ConvertToUTF8(L"宽度").c_str(), &width, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"高度").c_str(), &height, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"深度").c_str(), &depth, 0.1f, 0.01f, 10.0f))
                {
                    Geometry::UpdateCube(primitive.mesh, width, height, depth);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::CYLINDER: {
                auto &radius = primitive.params.cylinder.radius;
                auto &height = primitive.params.cylinder.height;
                auto &segments = primitive.params.cylinder.segments;
                if (ImGui::DragFloat(ConvertToUTF8(L"半径").c_str(), &radius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"高度").c_str(), &height, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragInt(ConvertToUTF8(L"分段数").c_str(), &segments, 1, 3, 100))
                {
                    Geometry::UpdateCylinder(primitive.mesh, radius, height, segments);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::CONE: {
                auto &radius = primitive.params.cone.radius;
                auto &height = primitive.params.cone.height;
                auto &segments = primitive.params.cone.segments;
                if (ImGui::DragFloat(ConvertToUTF8(L"半径").c_str(), &radius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"高度").c_str(), &height, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragInt(ConvertToUTF8(L"分段数").c_str(), &segments, 1, 3, 100))
                {
                    Geometry::UpdateCone(primitive.mesh, radius, height, segments);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::PRISM: {
                auto &sides = primitive.params.prism.sides;
                auto &radius = primitive.params.prism.radius;
                auto &height = primitive.params.prism.height;
                if (ImGui::DragInt(ConvertToUTF8(L"边数").c_str(), &sides, 1, 3, 20) || ImGui::DragFloat(ConvertToUTF8(L"半径").c_str(), &radius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"高度").c_str(), &height, 0.1f, 0.01f, 10.0f))
                {
                    Geometry::UpdatePrism(primitive.mesh, sides, radius, height);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::PYRAMID: {
                auto &sides = primitive.params.pyramid.sides;
                auto &radius = primitive.params.pyramid.radius;
                auto &height = primitive.params.pyramid.height;
                if (ImGui::DragInt(ConvertToUTF8(L"边数").c_str(), &sides, 1, 3, 20) || ImGui::DragFloat(ConvertToUTF8(L"半径").c_str(), &radius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"高度").c_str(), &height, 0.1f, 0.01f, 10.0f))
                {
                    Geometry::UpdatePyramid(primitive.mesh, sides, radius, height);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::TORUS: {
                auto &majorRadius = primitive.params.torus.majorRadius;
                auto &minorRadius = primitive.params.torus.minorRadius;
                auto &majorSegments = primitive.params.torus.majorSegments;
                auto &minorSegments = primitive.params.torus.minorSegments;
                if (ImGui::DragFloat(ConvertToUTF8(L"大半径").c_str(), &majorRadius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"小半径").c_str(), &minorRadius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragInt(ConvertToUTF8(L"环段数").c_str(), &majorSegments, 1, 3, 100) ||
                    ImGui::DragInt(ConvertToUTF8(L"管段数").c_str(), &minorSegments, 1, 3, 100))
                {
                    Geometry::UpdateTorus(primitive.mesh, majorRadius, minorRadius, majorSegments, minorSegments);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::ELLIPSOID: {
                auto &xRadius = primitive.params.ellipsoid.radiusX;
                auto &yRadius = primitive.params.ellipsoid.radiusY;
                auto &zRadius = primitive.params.ellipsoid.radiusZ;
                auto &segments = primitive.params.ellipsoid.segments;
                if (ImGui::DragFloat(ConvertToUTF8(L"X半径").c_str(), &xRadius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"Y半径").c_str(), &yRadius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"Z半径").c_str(), &zRadius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragInt(ConvertToUTF8(L"分段数").c_str(), &segments, 1, 3, 100))
                {
                    Geometry::UpdateEllipsoid(primitive.mesh, xRadius, yRadius, zRadius, segments);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::FRUSTUM: {
                auto &topRadius = primitive.params.frustum.radiusTop;
                auto &bottomRadius = primitive.params.frustum.radiusBottom;
                auto &height = primitive.params.frustum.height;
                auto &segments = primitive.params.frustum.segments;
                if (ImGui::DragFloat(ConvertToUTF8(L"上底半径").c_str(), &topRadius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"下底半径").c_str(), &bottomRadius, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"高度").c_str(), &height, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragInt(ConvertToUTF8(L"分段数").c_str(), &segments, 1, 3, 100))
                {
                    Geometry::UpdateFrustum(primitive.mesh, topRadius, bottomRadius, height, segments);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::ARROW: {
                // 箭头的长度和半径
                auto &length = primitive.params.arrow.length;
                auto &radius = primitive.params.arrow.headSize;
                if (ImGui::DragFloat(ConvertToUTF8(L"箭头长度").c_str(), &length, 0.1f, 0.01f, 10.0f) ||
                    ImGui::DragFloat(ConvertToUTF8(L"箭头半径").c_str(), &radius, 0.1f, 0.01f, 10.0f))
                {
                    Geometry::UpdateArrow(primitive.mesh, length, radius);
                    primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
                }
                break;
            }
            case Geometry::END:
            default:
                break;
            }
            primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
            // 材质编辑器
            ShowMaterialEditor(*primitive.mesh->GetMaterial());
        }
        else
        {
            // 显示光源属性
            auto light = renderer->GetLights()[selectedObjectIndex - modelCount - primitiveCount];
            OnLightInspectorGUI(*light);
        }

        // delete
        if (ImGui::Button(ConvertToUTF8(L"删除").c_str()))
        {
            if (selectedObjectIndex >= 0)
            {
                // 删除选中的对象
                renderer->DeleteObject(selectedObjectIndex);
                selectedObjectIndex = -1;
            }
        }
    }

    ImGui::End();
}

void EditorUI::ShowMaterialEditor(Material &material)
{
    int materialType = static_cast<int>(material.type);
    const char *material_items[] = {"Blinn-Phong", "PBR"};
    if (ImGui::Combo(ConvertToUTF8(L"材质类型").c_str(), &materialType, material_items, IM_ARRAYSIZE(material_items)))
    {
        MaterialType oldType = material.type;
        MaterialType newType = static_cast<MaterialType>(materialType);
        
        // 类型改变时的处理
        material.type = newType;
        
        // 如果类型发生了实际变化，设置合适的默认值
        if (oldType != newType)
        {
            if (newType == PBR)
            {
                // 从Blinn-Phong切换到PBR时，设置合理的PBR默认值
                if (material.albedo == glm::vec3(0.0f))
                {
                    material.albedo = glm::vec3(0.8f, 0.8f, 0.8f);
                }
                if (material.metallic == 0.0f && material.roughness == 0.0f)
                {
                    material.metallic = 0.1f;
                    material.roughness = 0.5f;
                }
                if (material.ao == 0.0f)
                {
                    material.ao = 1.0f;
                }
            }
            else if (newType == BLINN_PHONG)
            {
                // 从PBR切换到Blinn-Phong时，设置合理的Blinn-Phong默认值
                if (material.diffuse == glm::vec3(0.0f))
                {
                    material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
                }
                if (material.specular == glm::vec3(0.0f))
                {
                    material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
                }
                if (material.shininess == 0.0f)
                {
                    material.shininess = 32.0f;
                }
            }
        }
    }

    if (material.type == BLINN_PHONG)
    {
        ImGui::ColorEdit3(ConvertToUTF8(L"漫反射").c_str(), glm::value_ptr(material.diffuse));
        ImGui::ColorEdit3(ConvertToUTF8(L"高光").c_str(), glm::value_ptr(material.specular));
        ImGui::DragFloat(ConvertToUTF8(L"高光系数").c_str(), &material.shininess, 1.0f, 1.0f, 256.0f);

        // 贴图选择器
        TextureSelector(ConvertToUTF8(L"漫反射贴图").c_str(), material.diffuseMap, "diffuse");
        TextureSelector(ConvertToUTF8(L"高光贴图").c_str(), material.specularMap, "specular");
        TextureSelector(ConvertToUTF8(L"法线贴图").c_str(), material.normalMap, "normal");

        material.useDiffuseMap = material.diffuseMap != nullptr;
        material.useSpecularMap = material.specularMap != nullptr;
        material.useNormalMap = material.normalMap != nullptr;
    }
    else
    {
        ImGui::ColorEdit3(ConvertToUTF8(L"反照率").c_str(), glm::value_ptr(material.albedo));
        ImGui::DragFloat(ConvertToUTF8(L"金属度").c_str(), &material.metallic, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat(ConvertToUTF8(L"粗糙度").c_str(), &material.roughness, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat(ConvertToUTF8(L"环境光遮蔽").c_str(), &material.ao, 0.01f, 0.0f, 1.0f);

        // 贴图选择器
        TextureSelector(ConvertToUTF8(L"反照率贴图").c_str(), material.albedoMap, "albedo");
        TextureSelector(ConvertToUTF8(L"金属度贴图").c_str(), material.metallicMap, "metallic");
        TextureSelector(ConvertToUTF8(L"粗糙度贴图").c_str(), material.roughnessMap, "roughness");
        TextureSelector(ConvertToUTF8(L"环境光遮蔽贴图").c_str(), material.aoMap, "ao");
        TextureSelector(ConvertToUTF8(L"法线贴图").c_str(), material.normalMap, "normal");
        material.useAlbedoMap = material.albedoMap != nullptr;
        material.useMetallicMap = material.metallicMap != nullptr;
        material.useRoughnessMap = material.roughnessMap != nullptr;
        material.useAOMap = material.aoMap != nullptr;
        material.useNormalMap = material.normalMap != nullptr;
    }
}
// 现存的所有Material编辑器，可以直接选择材质进行更改
void EditorUI::ShowMaterialEditor()
{
    ImGui::Begin(ConvertToUTF8(L"材质编辑器").c_str());

    // Example: If you want to show all materials from the renderer
    const auto &allMaterials = renderer->getALLMaterials();
    for (auto &material : allMaterials)
    {
        ImGui::PushID(std::hash<std::shared_ptr<Material>>{}(material));
        ImGui::Text("%s: %s", ConvertToUTF8(L"材质").c_str(), material->name.c_str());
        if (ImGui::TreeNode(material->name.c_str()))
        {
            ShowMaterialEditor(*material);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    ImGui::End();
}

void EditorUI::TextureSelector(const std::string &label, std::shared_ptr<Texture> &texture, const std::string &idSuffix)
{
    ImGui::PushID((label + idSuffix).c_str());
    
    // 标题行
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label.c_str());
    
    // 纹理预览区域
    ImGui::BeginGroup();
    {
        // 预览图像
        ImVec2 imageSize(64, 64);
        if (texture && texture->GetID() != 0)
        {
            ImGui::Image((void*)(intptr_t)texture->GetID(), imageSize);
            
            // 纹理信息
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Text("ID: %u", texture->GetID());
            if (!texture->GetPath().empty())
            {
                std::string filename = std::filesystem::path(texture->GetPath()).filename().string();
                ImGui::TextWrapped("%s", filename.c_str());
            }
            ImGui::EndGroup();
        }
        else
        {
            // 占位符
            ImGui::BeginChild("TexturePlaceholder", imageSize, true);
            ImGui::SetCursorPos(ImVec2(imageSize.x * 0.5f - 10, imageSize.y * 0.5f - 10));
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[IMG]");
            ImGui::EndChild();
            
            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", ConvertToUTF8(L"未分配贴图").c_str());
        }
    }
    ImGui::EndGroup();
    
    // 控制按钮
    ImGui::Spacing();
    
    // 选择按钮
    if (DrawButton(ConvertToUTF8(L"[选择]").c_str(), ImVec2(80, 0)))
    {
        std::string path = FileDialog::OpenFile(
            ConvertToUTF8(L"选择贴图").c_str(),
            "Image Files\0*.jpg;*.jpeg;*.png;*.tga;*.bmp;*.hdr;*.exr\0All Files\0*.*\0"
        );
        if (!path.empty())
        {
            if (!texture)
                texture = std::make_shared<Texture>();
                
            // 保持翻转设置
            bool prevFlip = texture->flipY;
            if (!texture->LoadFromFile(path))
            {
                texture.reset();
                Application::AddConsoleLog(ConvertToUTF8(L"贴图加载失败: ") + path);
            }
            else
            {
                texture->flipY = prevFlip;
            }
        }
    }
    
    // 清除按钮
    ImGui::SameLine();
    if (DrawButton(ConvertToUTF8(L"[清除]").c_str(), ImVec2(80, 0)))
    {
        texture.reset();
    }
    
    // 设置选项
    if (texture)
    {
        ImGui::Spacing();
        
        // 翻转Y轴选项
        bool flip = texture->flipY;
        if (ImGui::Checkbox(ConvertToUTF8(L"翻转Y轴").c_str(), &flip))
        {
            texture->flipY = flip;
            if (!texture->GetPath().empty())
            {
                texture->LoadFromFile(texture->GetPath());
            }
        }
        DrawTooltip(ConvertToUTF8(L"某些贴图可能需要翻转Y轴以正确显示").c_str());
        
        // 拖拽支持
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("TEXTURE", &texture, sizeof(std::shared_ptr<Texture>));
            ImGui::Text("%s", label.c_str());
            ImGui::EndDragDropSource();
        }
    }
    
    // 拖拽目标
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_ITEM"))
        {
            size_t assetIndex = *(const size_t*)payload->Data;
            if (assetIndex < assetItems.size())
            {
                const auto& asset = assetItems[assetIndex];
                if (asset.type == AssetType::TEXTURE)
                {
                    if (!texture)
                        texture = std::make_shared<Texture>();
                    texture->LoadFromFile(asset.path.string());
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
    
    ImGui::PopID();
    ImGui::Spacing();
}

void EditorUI::ShowRendererSettings()
{
    ImGui::Begin(ConvertToUTF8(L"渲染器设置").c_str(), &showRendererSettings);

    // 使用折叠树来组织设置
    if (ImGui::CollapsingHeader(ConvertToUTF8(L"基础渲染").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        // 渲染模式
        static const char *renderModes[] = {"Forward", "Deferred"};
        int currentMode = static_cast<int>(renderer->GetRenderMode());
        if (ImGui::Combo(ConvertToUTF8(L"渲染模式").c_str(), &currentMode, renderModes, IM_ARRAYSIZE(renderModes)))
        {
            renderer->SetRenderMode(static_cast<Renderer::RenderMode>(currentMode));
        }
        
        // 伽马校正
        bool gamma = renderer->IsGammaCorrectionEnabled();
        if (ImGui::Checkbox(ConvertToUTF8(L"伽马校正").c_str(), &gamma))
        {
            renderer->SetGammaCorrection(gamma);
        }
        DrawTooltip(ConvertToUTF8(L"启用伽马校正以获得更准确的颜色表现").c_str());
    }

    // 抗锯齿设置
    if (ImGui::CollapsingHeader(ConvertToUTF8(L"抗锯齿").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ShowAntiAliasingSettings();
    }

    // 后处理效果
    if (ImGui::CollapsingHeader(ConvertToUTF8(L"后处理效果").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ShowPostProcessSettings();
    }

    // 光照设置
    if (ImGui::CollapsingHeader(ConvertToUTF8(L"光照").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ShowLightingSettings();
    }

    // 阴影设置
    if (ImGui::CollapsingHeader(ConvertToUTF8(L"阴影").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ShowShadowSettings();
    }

    ImGui::End();
}

void EditorUI::ShowAntiAliasingSettings()
{
    static std::string noneText = ConvertToUTF8(L"无");
    static std::string msaa2xText = "MSAA 2x";
    static std::string msaa4xText = "MSAA 4x";
    static std::string msaa8xText = "MSAA 8x";
    static std::string msaa16xText = "MSAA 16x";
    static std::string fxaaText = "FXAA";
    
    const char* aaOptions[] = {
        noneText.c_str(),
        msaa2xText.c_str(),
        msaa4xText.c_str(),
        msaa8xText.c_str(),
        msaa16xText.c_str(),
        fxaaText.c_str()
    };
    
    int currentAA = static_cast<int>(currentAAType);
    if (ImGui::Combo(ConvertToUTF8(L"抗锯齿类型").c_str(), &currentAA, aaOptions, IM_ARRAYSIZE(aaOptions)))
    {
        currentAAType = static_cast<AntiAliasingType>(currentAA);
        
        // 更新渲染器设置
        switch (currentAAType)
        {
            case AntiAliasingType::NONE:
                renderer->SetMSAA(false);
                renderer->SetFXAA(false);
                msaaSamples = 0;
                AddNotification(ConvertToUTF8(L"抗锯齿已禁用"), true, 2.0f);
                break;
            case AntiAliasingType::MSAA_2X:
                renderer->SetMSAA(true, 2);
                renderer->SetFXAA(false);
                msaaSamples = 2;
                AddNotification(ConvertToUTF8(L"MSAA 2x 已启用"), true, 2.0f);
                break;
            case AntiAliasingType::MSAA_4X:
                renderer->SetMSAA(true, 4);
                renderer->SetFXAA(false);
                msaaSamples = 4;
                AddNotification(ConvertToUTF8(L"MSAA 4x 已启用"), true, 2.0f);
                break;
            case AntiAliasingType::MSAA_8X:
                renderer->SetMSAA(true, 8);
                renderer->SetFXAA(false);
                msaaSamples = 8;
                AddNotification(ConvertToUTF8(L"MSAA 8x 已启用"), true, 2.0f);
                break;
            case AntiAliasingType::MSAA_16X:
                renderer->SetMSAA(true, 16);
                renderer->SetFXAA(false);
                msaaSamples = 16;
                AddNotification(ConvertToUTF8(L"MSAA 16x 已启用（高性能需求）"), true, 3.0f);
                break;
            case AntiAliasingType::FXAA:
                renderer->SetMSAA(false);
                renderer->SetFXAA(true);
                msaaSamples = 0;
                AddNotification(ConvertToUTF8(L"FXAA 已启用"), true, 2.0f);
                break;
        }
    }
    
    ImGui::Separator();
    
    // 显示当前状态
    ImGui::TextUnformatted(ConvertToUTF8(L"当前状态:").c_str());
    ImGui::Indent();
    
    // MSAA状态
    std::string msaaStatus = renderer->IsMSAAEnabled() ? ConvertToUTF8(L"启用") : ConvertToUTF8(L"禁用");
    ImGui::Text("MSAA: %s", msaaStatus.c_str());
    if (renderer->IsMSAAEnabled())
    {
        ImGui::SameLine();
        ImGui::Text("(%dx)", renderer->GetMSAASamples());
    }
    
    // FXAA状态
    std::string fxaaStatus = renderer->IsFXAAEnabled() ? ConvertToUTF8(L"启用") : ConvertToUTF8(L"禁用");
    ImGui::Text("FXAA: %s", fxaaStatus.c_str());
    
    ImGui::Unindent();
    
    // MSAA详细设置（仅在MSAA启用时显示）
    if (renderer->IsMSAAEnabled())
    {
        ImGui::Separator();
        ImGui::TextUnformatted(ConvertToUTF8(L"MSAA 设置:").c_str());
        
        // 显示支持的最大采样数
        GLint maxSamples;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        ImGui::Text(ConvertToUTF8(L"硬件支持最大采样数: %d").c_str(), maxSamples);
        
        // 性能提示
        if (renderer->GetMSAASamples() >= 8)
        {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ConvertToUTF8(L"高采样率可能影响性能").c_str());
        }
        
        // 快速切换按钮
        std::string quickSwitchText = ConvertToUTF8(L"快速切换:");
        ImGui::Text("%s", quickSwitchText.c_str());
        ImGui::SameLine();
        
        // 获取当前采样数以高亮显示对应按钮
        int currentSamples = renderer->GetMSAASamples();
        
        // 2x按钮
        if (currentSamples == 2) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 0.6f));
        }
        if (ImGui::SmallButton("2x")) {
            renderer->SetMSAA(true, 2);
            currentAAType = AntiAliasingType::MSAA_2X;
            msaaSamples = 2;
        }
        if (currentSamples == 2) {
            ImGui::PopStyleColor();
        }
        
        ImGui::SameLine();
        
        // 4x按钮
        if (currentSamples == 4) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 0.6f));
        }
        if (ImGui::SmallButton("4x")) {
            renderer->SetMSAA(true, 4);
            currentAAType = AntiAliasingType::MSAA_4X;
            msaaSamples = 4;
        }
        if (currentSamples == 4) {
            ImGui::PopStyleColor();
        }
        
        ImGui::SameLine();
        
        // 8x按钮
        if (currentSamples == 8) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 0.6f));
        }
        if (ImGui::SmallButton("8x")) {
            renderer->SetMSAA(true, 8);
            currentAAType = AntiAliasingType::MSAA_8X;
            msaaSamples = 8;
        }
        if (currentSamples == 8) {
            ImGui::PopStyleColor();
        }
        
        ImGui::SameLine();
        
        // 16x按钮（只在硬件支持时显示）
        if (maxSamples >= 16) {
            if (currentSamples == 16) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 0.6f));
            }
            if (ImGui::SmallButton("16x")) {
                renderer->SetMSAA(true, 16);
                currentAAType = AntiAliasingType::MSAA_16X;
                msaaSamples = 16;
            }
            if (currentSamples == 16) {
                ImGui::PopStyleColor();
            }
        } else {
            // 显示不可用的16x按钮
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
            ImGui::SmallButton("16x");
            ImGui::PopStyleVar();
            if (ImGui::IsItemHovered()) {
                std::string tooltip = ConvertToUTF8(L"硬件不支持16x采样");
                ImGui::SetTooltip("%s", tooltip.c_str());
            }
        }
    }
}

void EditorUI::ShowPostProcessSettings()
{
    bool hdr = renderer->IsHDREnabled();
    if (ImGui::Checkbox("HDR", &hdr))
    {
        renderer->SetHDR(hdr);
    }
    DrawTooltip(ConvertToUTF8(L"高动态范围渲染，支持更真实的光照效果").c_str());

    bool bloom = renderer->IsBloomEnabled();
    if (ImGui::Checkbox(ConvertToUTF8(L"泛光").c_str(), &bloom))
    {
        renderer->SetBloom(bloom);
    }
    DrawTooltip(ConvertToUTF8(L"为亮区添加发光效果").c_str());

    bool ssao = renderer->IsSSAOEnabled();
    if (ImGui::Checkbox("SSAO", &ssao))
    {
        renderer->SetSSAO(ssao);
    }
    DrawTooltip(ConvertToUTF8(L"屏幕空间环境光遮蔽，增强深度感").c_str());
}

void EditorUI::ShowLightingSettings()
{
    bool ibl = renderer->IsIBLEnabled();
    if (ImGui::Checkbox("IBL", &ibl))
    {
        renderer->SetIBL(ibl);
    }
    DrawTooltip(ConvertToUTF8(L"基于图像的光照，使用环境贴图").c_str());

    bool showLights = renderer->IsLightsEnabled();
    if (ImGui::Checkbox(ConvertToUTF8(L"显示光源").c_str(), &showLights))
    {
        renderer->SetLightsEnabled(showLights);
    }
    DrawTooltip(ConvertToUTF8(L"在场景中显示光源图标").c_str());
    
    ImGui::Separator();
    
    // 背景类型选择
    if (ImGui::CollapsingHeader(ConvertToUTF8(L"背景设置").c_str()))
    {
        const char* environmentNames[] = { "Skybox", "Newport Loft HDR", "Environment 2", "Environment 3", "Environment 4", "Environment 5", "Environment 6", "Environment 7", "Environment 8", "Environment 9" };
        int currentEnv = renderer->GetCurrentEnvironment();
        
        if (ImGui::Combo(ConvertToUTF8(L"环境类型").c_str(), &currentEnv, environmentNames, 10))
        {
            renderer->SetCurrentEnvironment(currentEnv);
        }
        DrawTooltip(ConvertToUTF8(L"选择当前环境贴图：天空盒或HDR环境").c_str());
        
        // 背景贴图gamma校正设置
        bool backgroundGamma = renderer->IsBackgroundGammaCorrectionEnabled();
        if (ImGui::Checkbox(ConvertToUTF8(L"背景伽马校正").c_str(), &backgroundGamma))
        {
            renderer->SetBackgroundGammaCorrection(backgroundGamma);
        }
        DrawTooltip(ConvertToUTF8(L"启用背景贴图伽马校正，关闭时将对背景贴图进行反伽马校正").c_str());
        
        ImGui::Spacing();
        
        // 环境贴图导入功能
        if (ImGui::Button(ConvertToUTF8(L"导入HDR环境贴图").c_str()))
        {
            auto hdrPath = FileDialog::OpenFile("Select HDR File", "HDR Files\0*.hdr\0All Files\0*.*\0");
            if (!hdrPath.empty())
            {
                // 寻找可用的环境槽位
                bool slotFound = false;
                for (int slot = 2; slot < 10; ++slot) // 从槽位2开始，保留0和1给默认环境
                {
                    if (!renderer->IsEnvironmentSlotLoaded(slot))
                    {
                        renderer->LoadEnvironmentHDR(slot, hdrPath);
                        renderer->SetCurrentEnvironment(slot);
                        slotFound = true;
                        break;
                    }
                }
                if (!slotFound)
                {
                    // 如果没有空槽位，覆盖最后一个槽位
                    renderer->LoadEnvironmentHDR(9, hdrPath);
                    renderer->SetCurrentEnvironment(9);
                }
            }
        }
        DrawTooltip(ConvertToUTF8(L"导入HDR文件作为环境贴图，支持IBL照明").c_str());
        
        ImGui::SameLine();
        
        if (ImGui::Button(ConvertToUTF8(L"导入天空盒").c_str()))
        {
            // 重置天空盒面路径
            for (auto& face : skyboxFaces) {
                face.clear();
            }
            showSkyboxImportDialog = true;
        }
        DrawTooltip(ConvertToUTF8(L"导入立方体贴图天空盒（需要6张图片）").c_str());
        
        // 天空盒导入弹窗
        if (showSkyboxImportDialog)
        {
            ImGui::OpenPopup(ConvertToUTF8(L"天空盒导入").c_str());
        }
        
        if (ImGui::BeginPopupModal(ConvertToUTF8(L"天空盒导入").c_str(), &showSkyboxImportDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted(ConvertToUTF8(L"请为立方体贴图的每个面选择图片文件").c_str());
            ImGui::Separator();
            
            // 定义6个面的名称
            std::vector<std::string> faceNames = {
                ConvertToUTF8(L"右面 (Right)"),
                ConvertToUTF8(L"左面 (Left)"),
                ConvertToUTF8(L"上面 (Top)"),
                ConvertToUTF8(L"下面 (Bottom)"),
                ConvertToUTF8(L"前面 (Front)"),
                ConvertToUTF8(L"后面 (Back)")
            };
            
            // 显示6个面的选择按钮
            for (int i = 0; i < 6; i++)
            {
                ImGui::PushID(i);
                
                // 面名称标签
                ImGui::TextUnformatted(faceNames[i].c_str());
                ImGui::SameLine();
                
                // 选择文件按钮
                if (ImGui::Button(ConvertToUTF8(L"选择文件").c_str(), ImVec2(80, 0)))
                {
                    auto filePath = FileDialog::OpenFile("Select Skybox Face", "Image Files\0*.jpg;*.png;*.bmp;*.tga;*.hdr\0All Files\0*.*\0");
                    if (!filePath.empty())
                    {
                        skyboxFaces[i] = filePath;
                    }
                }
                
                ImGui::SameLine();
                
                // 显示已选择的文件名或提示
                if (!skyboxFaces[i].empty())
                {
                    std::filesystem::path p(skyboxFaces[i]);
                    ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "%s", p.filename().string().c_str());
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "%s", ConvertToUTF8(L"未选择").c_str());
                }
                
                ImGui::PopID();
            }
            
            ImGui::Separator();
            
            // 检查是否所有面都已选择
            bool allFacesSelected = true;
            for (const auto& face : skyboxFaces)
            {
                if (face.empty())
                {
                    allFacesSelected = false;
                    break;
                }
            }
            
            // 导入按钮
            if (!allFacesSelected)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
            }
            
            if (ImGui::Button(ConvertToUTF8(L"导入天空盒").c_str(), ImVec2(120, 0)) && allFacesSelected)
            {
                // 寻找可用的环境槽位
                bool slotFound = false;
                for (int slot = 2; slot < 10; ++slot) // 从槽位2开始，保留0和1给默认环境
                {
                    if (!renderer->IsEnvironmentSlotLoaded(slot))
                    {
                        renderer->LoadEnvironmentSkybox(slot, skyboxFaces);
                        renderer->SetCurrentEnvironment(slot);
                        slotFound = true;
                        showSkyboxImportDialog = false;
                        break;
                    }
                }
                
                if (!slotFound)
                {
                    // 如果没有空槽位，覆盖最后一个槽位
                    renderer->LoadEnvironmentSkybox(9, skyboxFaces);
                    renderer->SetCurrentEnvironment(9);
                    showSkyboxImportDialog = false;
                }
            }
            
            if (!allFacesSelected)
            {
                ImGui::PopStyleVar();
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "%s", ConvertToUTF8(L"请选择所有6个面").c_str());
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button(ConvertToUTF8(L"取消").c_str(), ImVec2(80, 0)))
            {
                showSkyboxImportDialog = false;
            }
            
            ImGui::EndPopup();
        }
        
        ImGui::Spacing();
        
        // 显示当前环境状态
        if (currentEnv == 0) // Skybox
        {
            ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "Using traditional skybox");
        }
        else if (currentEnv == 1) // Newport Loft HDR
        {
            ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.4f, 1.0f), "Using Newport Loft HDR");
        }
        else
        {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), ConvertToUTF8(L"环境槽位 %d").c_str(), currentEnv);
        }
    }
}

void EditorUI::ShowShadowSettings()
{
    bool shadow = renderer->IsShadowEnabled();
    if (ImGui::Checkbox(ConvertToUTF8(L"阴影##ShadowSetting").c_str(), &shadow))
    {
        renderer->SetShadow(shadow);
    }
    DrawTooltip(ConvertToUTF8(L"启用实时阴影渲染").c_str());
}

// 实用工具函数
void EditorUI::DrawSeparator()
{
    ImGui::Separator();
}

void EditorUI::DrawTooltip(const char* text)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool EditorUI::DrawButton(const char* label, const ImVec2& size)
{
    return ImGui::Button(label, size);
}

void EditorUI::DrawTextCentered(const char* text)
{
    float windowWidth = ImGui::GetWindowSize().x;
    float textWidth = ImGui::CalcTextSize(text).x;
    
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text);
}

void EditorUI::EndFrame()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorUI::OnLightInspectorGUI(Light &light)
{
    if (light.getType() == 0)
    {
        auto &pointLight = static_cast<PointLight &>(light);
        ImGui::Text("%s %d", ConvertToUTF8(L"点光源").c_str(), pointLight.number);
        ImGui::DragFloat3(ConvertToUTF8(L"位置").c_str(), glm::value_ptr(pointLight.position), 0.1f);
        ImGui::ColorEdit3(ConvertToUTF8(L"环境光").c_str(), glm::value_ptr(pointLight.ambient));
        ImGui::ColorEdit3(ConvertToUTF8(L"漫反射").c_str(), glm::value_ptr(pointLight.diffuse));
        ImGui::ColorEdit3(ConvertToUTF8(L"高光").c_str(), glm::value_ptr(pointLight.specular));
        ImGui::SliderFloat(ConvertToUTF8(L"强度").c_str(), &pointLight.intensity, 0.0f, 1.0f);
        ImGui::SliderFloat(ConvertToUTF8(L"常数项").c_str(), &pointLight.constant, 0.0f, 1.0f);
        ImGui::SliderFloat(ConvertToUTF8(L"线性项").c_str(), &pointLight.linear, 0.0f, 1.0f);
        ImGui::SliderFloat(ConvertToUTF8(L"二次项").c_str(), &pointLight.quadratic, 0.0f, 1.0f);
        
        // 阴影设置
        bool shadowEnabled = pointLight.IsShadowEnabled();
        if (ImGui::Checkbox(ConvertToUTF8(L"启用阴影").c_str(), &shadowEnabled))
        {
            pointLight.SetShadowEnabled(shadowEnabled);
        }
        
        if (shadowEnabled)
        {
            ImGui::DragFloat(ConvertToUTF8(L"阴影近平面").c_str(), &pointLight.shadowNearPlane, 0.1f, 0.1f, 10.0f);
            ImGui::DragFloat(ConvertToUTF8(L"阴影远平面").c_str(), &pointLight.shadowFarPlane, 1.0f, 10.0f, 1000.0f);
        }
    }
    else if (light.getType() == 1)
    {
        auto &directionalLight = static_cast<DirectionalLight &>(light);
        ImGui::Text("%s %d", ConvertToUTF8(L"方向光源").c_str(), directionalLight.number);
        ImGui::DragFloat3(ConvertToUTF8(L"方向").c_str(), glm::value_ptr(directionalLight.direction), 0.1f,- 1.0f, 1.0f);
        //directionalLight.direction = glm::normalize(directionalLight.direction);
        ImGui::Text("%s: (0.0, 0.0, 0.0)", ConvertToUTF8(L"位置").c_str()); // 定向光没有位置
        ImGui::ColorEdit3(ConvertToUTF8(L"环境光").c_str(), glm::value_ptr(directionalLight.ambient));
        ImGui::ColorEdit3(ConvertToUTF8(L"漫反射").c_str(), glm::value_ptr(directionalLight.diffuse));
        ImGui::ColorEdit3(ConvertToUTF8(L"高光").c_str(), glm::value_ptr(directionalLight.specular));
        ImGui::SliderFloat(ConvertToUTF8(L"强度").c_str(), &directionalLight.intensity, 0.0f, 1.0f);
        
        // 阴影设置
        bool shadowEnabled = directionalLight.IsShadowEnabled();
        if (ImGui::Checkbox(ConvertToUTF8(L"启用阴影").c_str(), &shadowEnabled))
        {
            directionalLight.SetShadowEnabled(shadowEnabled);
        }
        
        if (shadowEnabled)
        {
            ImGui::DragFloat(ConvertToUTF8(L"阴影近平面").c_str(), &directionalLight.shadowNearPlane, 0.1f, 0.1f, 10.0f);
            ImGui::DragFloat(ConvertToUTF8(L"阴影远平面").c_str(), &directionalLight.shadowFarPlane, 1.0f, 10.0f, 1000.0f);
            ImGui::DragFloat(ConvertToUTF8(L"阴影正交大小").c_str(), &directionalLight.shadowOrthoSize, 1.0f, 5.0f, 100.0f);
        }
    }
    else if (light.getType() == 2)
    {
        auto &spotLight = static_cast<SpotLight &>(light);
        ImGui::Text("%s", ConvertToUTF8(L"聚光灯").c_str());
        ImGui::DragFloat3(ConvertToUTF8(L"位置").c_str(), glm::value_ptr(spotLight.position), 0.1f);
        
        // 方向输入，自动归一化
        if (ImGui::DragFloat3(ConvertToUTF8(L"方向").c_str(), glm::value_ptr(spotLight.direction), 0.01f, -1.0f, 1.0f)) {
            // 在方向改变时自动归一化，避免零向量
            if (glm::length(spotLight.direction) > 0.001f) {
                spotLight.direction = glm::normalize(spotLight.direction);
            } else {
                spotLight.direction = glm::vec3(0.0f, -1.0f, 0.0f); // 默认向下
            }
        }
        
        ImGui::ColorEdit3(ConvertToUTF8(L"环境光").c_str(), glm::value_ptr(spotLight.ambient));
        ImGui::ColorEdit3(ConvertToUTF8(L"漫反射").c_str(), glm::value_ptr(spotLight.diffuse));
        ImGui::ColorEdit3(ConvertToUTF8(L"高光").c_str(), glm::value_ptr(spotLight.specular));
        ImGui::DragFloat(ConvertToUTF8(L"强度").c_str(), &spotLight.intensity, 0.1f, 0.0f, 100.0f);

        // 内切角应该小于外切角
        float degrees = glm::degrees(glm::acos(spotLight.cutOff));
        float outerDegrees = glm::degrees(glm::acos(spotLight.outerCutOff));
        // 内切角是光锥的角度，范围在0到90度之间
        if (ImGui::DragFloat(ConvertToUTF8(L"内切角").c_str(), &degrees, 1.0f, 0.0f, outerDegrees))
        {
            spotLight.cutOff = glm::cos(glm::radians(degrees));
        }

        // 外切角应该大于内切角
        if (ImGui::DragFloat(ConvertToUTF8(L"外切角").c_str(), &outerDegrees, 1.0f, degrees, 89.9f))
        {
            spotLight.outerCutOff = glm::cos(glm::radians(outerDegrees));
        }

        ImGui::DragFloat(ConvertToUTF8(L"常数项").c_str(), &spotLight.constant, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat(ConvertToUTF8(L"线性项").c_str(), &spotLight.linear, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat(ConvertToUTF8(L"二次项").c_str(), &spotLight.quadratic, 0.0001f, 0.0f, 1.0f);
        
        // 阴影设置
        bool shadowEnabled = spotLight.IsShadowEnabled();
        if (ImGui::Checkbox(ConvertToUTF8(L"启用阴影").c_str(), &shadowEnabled))
        {
            spotLight.SetShadowEnabled(shadowEnabled);
        }
        
        if (shadowEnabled)
        {
            ImGui::DragFloat(ConvertToUTF8(L"阴影近平面").c_str(), &spotLight.shadowNearPlane, 0.1f, 0.1f, 10.0f);
            ImGui::DragFloat(ConvertToUTF8(L"阴影远平面").c_str(), &spotLight.shadowFarPlane, 1.0f, 10.0f, 1000.0f);
        }
    }
}

// 通知系统实现
void EditorUI::AddNotification(const std::string& message, bool isSuccess, float duration)
{
    notifications.push_back({message, duration, 0.0f, isSuccess});
}

void EditorUI::UpdateNotifications(float deltaTime)
{
    for (auto it = notifications.begin(); it != notifications.end();) {
        it->timer += deltaTime;
        if (it->timer >= it->duration) {
            it = notifications.erase(it);
        } else {
            ++it;
        }
    }
}

void EditorUI::DrawNotifications()
{
    if (notifications.empty()) return;
    
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    
    float notificationWidth = 300.0f;
    float notificationHeight = 50.0f;
    float rightMargin = 20.0f;
    float topMargin = 80.0f; // 留出菜单栏空间
    
    for (size_t i = 0; i < notifications.size(); ++i) {
        const auto& notification = notifications[i];
        
        // 计算位置（从上往下排列）
        float posX = displaySize.x - notificationWidth - rightMargin;
        float posY = topMargin + i * (notificationHeight + 10.0f);
        
        // 设置窗口位置和大小
        ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(notificationWidth, notificationHeight), ImGuiCond_Always);
        
        // 创建无标题栏、无调整大小的窗口
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
        
        // 设置背景颜色
        ImGui::PushStyleColor(ImGuiCol_WindowBg, notification.isSuccess ? 
                             ImVec4(0.2f, 0.7f, 0.2f, 0.9f) : ImVec4(0.7f, 0.2f, 0.2f, 0.9f));
        
        std::string windowName = "Notification##" + std::to_string(i);
        if (ImGui::Begin(windowName.c_str(), nullptr, flags)) {
            // 计算淡出效果
            float alpha = 1.0f;
            float fadeTime = 1.0f; // 最后1秒开始淡出
            if (notification.timer > notification.duration - fadeTime) {
                alpha = (notification.duration - notification.timer) / fadeTime;
            }
            
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            
            // 居中显示文本
            ImVec2 textSize = ImGui::CalcTextSize(notification.message.c_str());
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImGui::SetCursorPos(ImVec2(
                (windowSize.x - textSize.x) * 0.5f,
                (windowSize.y - textSize.y) * 0.5f
            ));
            
            ImGui::Text("%s", notification.message.c_str());
            
            ImGui::PopStyleVar(); // Alpha
        }
        ImGui::End();
        
        ImGui::PopStyleColor(); // WindowBg
    }
}

// 资源预加载函数
void EditorUI::PreloadAsset(AssetItem &item)
{
    if (item.isPreloaded) return;
    
    // 检查是否已经在缓存中
    std::string key = item.path.string();
    if (preloadedAssets.find(key) != preloadedAssets.end())
    {
        item.isPreloaded = true;
        return;
    }
    
    // 根据资源类型进行预加载
    try 
    {
        switch (item.type)
        {
            case AssetType::TEXTURE:
            {
                auto texture = std::make_shared<Texture>();
                if (texture->LoadFromFile(item.path.string()))
                {
                    item.resource = texture;
                    item.previewTexture = texture;
                    // 注意：由于Width, Height, nrComponents是私有成员，这里需要添加公有访问器
                    // 或者从文件中重新读取图像信息
                    // 暂时设置默认值
                    item.cachedData.textureData.width = 512;
                    item.cachedData.textureData.height = 512;
                    item.cachedData.textureData.channels = 4;
                    preloadedAssets[key] = texture;
                    item.isPreloaded = true;
                    AddNotification(ConvertToUTF8(L"预加载纹理: ") + item.name, true, 2.0f);
                }
                break;
            }
            case AssetType::MODEL:
            {
                // 为模型创建缩略图纹理（简化实现）
                // 实际项目中可能需要离屏渲染生成模型预览
                item.isPreloaded = true;
                AddNotification(ConvertToUTF8(L"预加载模型: ") + item.name, true, 2.0f);
                break;
            }
            default:
                break;
        }
    }
    catch (const std::exception& e)
    {
        AddNotification(ConvertToUTF8(L"预加载失败: ") + item.name + " - " + e.what(), false, 3.0f);
    }
}

void EditorUI::UnloadAsset(AssetItem &item)
{
    std::string key = item.path.string();
    auto it = preloadedAssets.find(key);
    if (it != preloadedAssets.end())
    {
        preloadedAssets.erase(it);
    }
    
    item.resource.reset();
    item.previewTexture.reset();
    item.isPreloaded = false;
}

void EditorUI::ShowAssetPreviewWindow()
{
    if (!selectedAsset) return;
    
    ImGui::Begin(ConvertToUTF8(L"资源预览").c_str(), &showAssetPreview, ImGuiWindowFlags_AlwaysAutoResize);
    
    // 资源信息
    ImGui::Text(ConvertToUTF8(L"名称: %s").c_str(), selectedAsset->name.c_str());
    ImGui::Text(ConvertToUTF8(L"路径: %s").c_str(), selectedAsset->path.string().c_str());

    const wchar_t* typeNames[] = { 
        L"纹理", 
        L"模型", 
        L"材质", 
        L"着色器", 
        L"音频", 
        L"未知"
    };
    ImGui::Text(ConvertToUTF8(L"类型: %s").c_str(), ConvertToUTF8(typeNames[(int)selectedAsset->type]).c_str());

    ImGui::Separator();
    
    // 预加载控制
    if (!selectedAsset->isPreloaded)
    {
        if (ImGui::Button(ConvertToUTF8(L"预加载").c_str()))
        {
            PreloadAsset(*selectedAsset);
        }
    }
    else
    {
        std::string text = ConvertToUTF8(L"已预加载");
        ImGui::Text("%s", text.c_str());
        ImGui::SameLine();
        if (ImGui::Button(ConvertToUTF8(L"卸载").c_str()))
        {
            UnloadAsset(*selectedAsset);
        }
    }
    
    ImGui::Separator();
    
    // 根据资源类型显示预览
    switch (selectedAsset->type)
    {
        case AssetType::TEXTURE:
            ShowTexturePreview(*selectedAsset);
            break;
        case AssetType::MODEL:
            ShowModelPreview(*selectedAsset);
            break;
        default:
        {
            std::string text = ConvertToUTF8(L"暂不支持此类型预览");
            ImGui::Text("%s", text.c_str());
            break;
        }
    }
    
    ImGui::Separator();
    
    // 操作按钮
    if (ImGui::Button(ConvertToUTF8(L"应用到选中对象").c_str()))
    {
        ApplyAssetToSelected(*selectedAsset);
    }
    ImGui::SameLine();
    if (ImGui::Button(ConvertToUTF8(L"添加到场景").c_str()))
    {
        if (selectedAsset->type == AssetType::MODEL)
        {
            renderer->LoadModel(selectedAsset->path.string());
            AddNotification(ConvertToUTF8(L"模型已添加到场景: ") + selectedAsset->name, true);
        }
    }
    
    ImGui::End();
}

void EditorUI::ShowTexturePreview(const AssetItem &item)
{
    if (item.previewTexture)
    {
        // 显示纹理信息
        ImGui::Text(ConvertToUTF8(L"尺寸: %dx%d").c_str(), 
                   item.cachedData.textureData.width, 
                   item.cachedData.textureData.height);
        ImGui::Text(ConvertToUTF8(L"通道数: %d").c_str(), item.cachedData.textureData.channels);
        
        ImGui::Separator();
        
        // 显示纹理预览
        float aspectRatio = (float)item.cachedData.textureData.width / item.cachedData.textureData.height;
        ImVec2 imageSize;
        if (aspectRatio > 1.0f)
        {
            imageSize.x = previewWindowSize;
            imageSize.y = previewWindowSize / aspectRatio;
        }
        else
        {
            imageSize.x = previewWindowSize * aspectRatio;
            imageSize.y = previewWindowSize;
        }
        
        ImGui::Image((void*)(intptr_t)item.previewTexture->GetID(), imageSize);
        
        // 缩放控制
        ImGui::SliderFloat(ConvertToUTF8(L"预览大小").c_str(), &previewWindowSize, 64.0f, 512.0f);
    }
    else
    {
        std::string text = ConvertToUTF8(L"纹理未加载");
        ImGui::Text("%s", text.c_str());
    }
}

void EditorUI::ShowModelPreview(const AssetItem &item)
{
    std::string title = ConvertToUTF8(L"模型预览");
    ImGui::Text("%s", title.c_str());
    
    if (item.isPreloaded)
    {
        ImGui::Text(ConvertToUTF8(L"顶点数: %d").c_str(), item.cachedData.modelData.vertexCount);
        ImGui::Text(ConvertToUTF8(L"面数: %d").c_str(), item.cachedData.modelData.faceCount);
        
        // 简单的模型信息显示
        // 实际项目中可以在这里显示3D模型的缩略图
        ImGui::BeginChild("ModelPreview", ImVec2(previewWindowSize, previewWindowSize), true);
        std::string previewText1 = ConvertToUTF8(L"[3D模型预览]");
        std::string previewText2 = ConvertToUTF8(L"(需要3D渲染实现)");
        ImGui::Text("%s", previewText1.c_str());
        ImGui::Text("%s", previewText2.c_str());
        ImGui::EndChild();
    }
    else
    {
        std::string text = ConvertToUTF8(L"模型未预加载");
        ImGui::Text("%s", text.c_str());
    }
}

void EditorUI::ApplyAssetToSelected(const AssetItem &item)
{
    if (selectedObjectIndex < 0)
    {
        AddNotification(ConvertToUTF8(L"请先选择一个对象"), false);
        return;
    }

    // 检查选中的对象类型
    int modelCount = renderer->GetModelCount();
    int primitiveCount = renderer->GetPrimitives().size();
    
    // 如果选中的是光源，不能应用材质
    if (selectedObjectIndex >= modelCount + primitiveCount)
    {
        AddNotification(ConvertToUTF8(L"无法将材质应用到光源"), false);
        return;
    }
    
    // 只处理纹理和材质类型的资源
    if (item.type != AssetType::TEXTURE && item.type != AssetType::MATERIAL)
    {
        AddNotification(ConvertToUTF8(L"此资源类型无法应用到对象"), false);
        return;
    }
    
    // 保存待应用的资源信息
    pendingAssetToApply = item;
    selectedMeshIndex = -1;
    selectedMaterialProperty = 0;
    
    // 显示材质应用对话框
    showMaterialApplicationDialog = true;
}

void EditorUI::ShowMaterialApplicationDialog()
{
    if (!showMaterialApplicationDialog)
        return;
        
    ImGui::OpenPopup(ConvertToUTF8(L"应用材质到对象").c_str());
    
    if (ImGui::BeginPopupModal(ConvertToUTF8(L"应用材质到对象").c_str(), &showMaterialApplicationDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        int modelCount = renderer->GetModelCount();
        int primitiveCount = renderer->GetPrimitives().size();
        
        // 确定选中对象的类型和信息
        bool isModel = selectedObjectIndex < modelCount;
        bool isPrimitive = selectedObjectIndex >= modelCount && selectedObjectIndex < modelCount + primitiveCount;
        
        ImGui::Text(ConvertToUTF8(L"资源: %s").c_str(), pendingAssetToApply.name.c_str());
        ImGui::Separator();
        
        if (isModel)
        {
            // 获取选中的模型
            auto models = renderer->GetModels();
            if (selectedObjectIndex < models.size())
            {
                auto model = models[selectedObjectIndex];
                auto meshes = model->GetMeshes();
                
                ImGui::Text("%s", ConvertToUTF8(L"选择要应用的Mesh:").c_str());
                
                // 显示所有可用的mesh
                for (int i = 0; i < meshes.size(); i++)
                {
                    ImGui::PushID(i);
                    if (ImGui::RadioButton((ConvertToUTF8(L"Mesh ") + std::to_string(i + 1)).c_str(), selectedMeshIndex == i))
                    {
                        selectedMeshIndex = i;
                    }
                    ImGui::PopID();
                }
                
                if (ImGui::RadioButton(ConvertToUTF8(L"应用到所有Mesh").c_str(), selectedMeshIndex == -1))
                {
                    selectedMeshIndex = -1;
                }
                
                ImGui::Separator();
            }
        }
        else if (isPrimitive)
        {
            ImGui::Text("%s", ConvertToUTF8(L"对象类型: 简单几何体").c_str());
            ImGui::Separator();
        }
        
        // 材质属性选择
        ImGui::Text("%s", ConvertToUTF8(L"选择要应用的材质属性:").c_str());
        
        if (pendingAssetToApply.type == AssetType::MATERIAL)
        {
            ImGui::RadioButton(ConvertToUTF8(L"整个材质").c_str(), &selectedMaterialProperty, 0);
        }
        else if (pendingAssetToApply.type == AssetType::TEXTURE)
        {
            ImGui::RadioButton(ConvertToUTF8(L"漫反射贴图").c_str(), &selectedMaterialProperty, 1);
            ImGui::RadioButton(ConvertToUTF8(L"法线贴图").c_str(), &selectedMaterialProperty, 2);
            ImGui::RadioButton(ConvertToUTF8(L"高光贴图").c_str(), &selectedMaterialProperty, 3);
            ImGui::RadioButton(ConvertToUTF8(L"粗糙度贴图").c_str(), &selectedMaterialProperty, 4);
            ImGui::RadioButton(ConvertToUTF8(L"金属度贴图").c_str(), &selectedMaterialProperty, 5);
            ImGui::RadioButton(ConvertToUTF8(L"环境光遮蔽贴图").c_str(), &selectedMaterialProperty, 6);
            ImGui::RadioButton(ConvertToUTF8(L"反照率贴图").c_str(), &selectedMaterialProperty, 7);
        }
        
        ImGui::Separator();
        
        // 应用和取消按钮
        if (ImGui::Button(ConvertToUTF8(L"应用").c_str(), ImVec2(120, 0)))
        {
            ApplyMaterialToObject();
            showMaterialApplicationDialog = false;
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button(ConvertToUTF8(L"取消").c_str(), ImVec2(120, 0)))
        {
            showMaterialApplicationDialog = false;
        }
        
        ImGui::EndPopup();
    }
}

void EditorUI::ApplyMaterialToObject()
{
    int modelCount = renderer->GetModelCount();
    int primitiveCount = renderer->GetPrimitives().size();
    
    bool isModel = selectedObjectIndex < modelCount;
    bool isPrimitive = selectedObjectIndex >= modelCount && selectedObjectIndex < modelCount + primitiveCount;
    
    std::string successMessage;
    
    if (isModel)
    {
        // 处理模型对象
        auto models = renderer->GetModels();
        if (selectedObjectIndex < models.size())
        {
            auto model = models[selectedObjectIndex];
            auto meshes = model->GetMeshes();
            
            if (selectedMeshIndex == -1)
            {
                // 应用到所有mesh
                for (auto& mesh : meshes)
                {
                    ApplyAssetToMesh(mesh);
                }
                successMessage = ConvertToUTF8(L"已应用到模型的所有Mesh: ") + pendingAssetToApply.name;
            }
            else if (selectedMeshIndex < meshes.size())
            {
                // 应用到指定mesh
                ApplyAssetToMesh(meshes[selectedMeshIndex]);
                successMessage = ConvertToUTF8(L"已应用到Mesh ") + std::to_string(selectedMeshIndex + 1) + ": " + pendingAssetToApply.name;
            }
        }
    }
    else if (isPrimitive)
    {
        // 处理简单几何体
        int primitiveIndex = selectedObjectIndex - modelCount;
        auto& primitives = renderer->GetPrimitives();
        
        if (primitiveIndex < primitives.size())
        {
            ApplyAssetToPrimitive(primitives[primitiveIndex]);
            successMessage = ConvertToUTF8(L"已应用到几何体: ") + pendingAssetToApply.name;
        }
    }
    
    if (!successMessage.empty())
    {
        AddNotification(successMessage, true);
    }
    else
    {
        AddNotification(ConvertToUTF8(L"应用材质失败"), false);
    }
}

void EditorUI::ApplyAssetToMesh(std::shared_ptr<Mesh> mesh)
{
    if (!mesh) return;
    
    if (pendingAssetToApply.type == AssetType::MATERIAL)
    {
        // TODO: 加载并应用整个材质
        // 这里需要实现材质文件的加载逻辑
    }
    else if (pendingAssetToApply.type == AssetType::TEXTURE)
    {
        // 获取mesh的当前材质，如果没有则创建一个新的
        auto material = mesh->GetMaterial();
        if (!material)
        {
            material = std::make_shared<Material>();
            mesh->SetMaterial(material);
        }
        
        // 根据选择的材质属性应用纹理
        auto texture = std::make_shared<Texture>(pendingAssetToApply.path.string());
        
        switch (selectedMaterialProperty)
        {
            case 1: // 漫反射贴图
                material->diffuseMap = texture;
                material->useDiffuseMap = true;
                break;
            case 2: // 法线贴图
                material->normalMap = texture;
                material->useNormalMap = true;
                break;
            case 3: // 高光贴图
                material->specularMap = texture;
                material->useSpecularMap = true;
                break;
            case 4: // 粗糙度贴图
                material->roughnessMap = texture;
                material->useRoughnessMap = true;
                break;
            case 5: // 金属度贴图
                material->metallicMap = texture;
                material->useMetallicMap = true;
                break;
            case 6: // 环境光遮蔽贴图
                material->aoMap = texture;
                material->useAOMap = true;
                break;
            case 7: // 反照率贴图
                material->albedoMap = texture;
                material->useAlbedoMap = true;
                break;
        }
    }
}

void EditorUI::ApplyAssetToPrimitive(Geometry::Primitive& primitive)
{
    if (pendingAssetToApply.type == AssetType::MATERIAL)
    {
        // TODO: 加载并应用整个材质到几何体
        // 这里需要实现材质文件的加载逻辑
    }
    else if (pendingAssetToApply.type == AssetType::TEXTURE)
    {
        // 对于简单几何体，我们通过其mesh来应用材质
        if (primitive.mesh)
        {
            ApplyAssetToMesh(primitive.mesh);
        }
    }
}