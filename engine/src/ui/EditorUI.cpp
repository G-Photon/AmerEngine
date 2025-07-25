#include "ui/EditorUI.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "utils/FileDialog.hpp"
#include "utils/FileSystem.hpp"
#include <cstdio>
#include <functional>
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
    io.Fonts->AddFontFromFileTTF(FileSystem::GetPath("resources/fonts/HarmonyOS_Sans_SC_Medium.ttf").c_str(), 16.0f,
                                 NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    io.ConfigFlags |= ImGuiConfigFlags_None;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

    // 1. 先加载 Dark 作为基底
    ImGui::StyleColorsDark();

    // 2. 拿到当前 style 的指针
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // 3. 把关键颜色改成黑红
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);             // 文字纯白
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);     // 禁用文字
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);         // 窗口背景—深黑
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);          // 子窗口背景
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);          // 弹出背景
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.00f, 0.00f, 0.50f);           // 边框暗红
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);     // 无边框阴影
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.00f, 0.00f, 0.54f);          // 输入框背景红
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.00f, 0.00f, 0.54f);   // 悬停更亮
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.60f, 0.00f, 0.00f, 0.67f);    // 激活最亮
    colors[ImGuiCol_TitleBg] = ImVec4(0.27f, 0.00f, 0.00f, 1.00f);          // 标题栏红
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.32f, 0.00f, 0.00f, 1.00f);    // 激活标题栏
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.27f, 0.00f, 0.00f, 0.51f); // 折叠标题栏
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.00f, 0.00f, 1.00f);        // 菜单栏红色
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);      // 滚动条背景黑
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.00f, 0.00f, 1.00f);    // 滚动条滑块红
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.00f, 0.00f, 1.00f); // 勾选红色
    colors[ImGuiCol_SliderGrab] = ImVec4(0.60f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.63f, 0.00f, 0.00f, 0.40f); // 按钮红
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.83f, 0.00f, 0.00f, 0.60f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.63f, 0.00f, 0.00f, 0.31f); // 折叠栏
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.83f, 0.00f, 0.00f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.00f, 0.00f, 0.50f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f); // 可忽略
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.00f, 0.00f, 0.86f);        // Tab 红
    colors[ImGuiCol_TabHovered] = ImVec4(0.36f, 0.00f, 0.00f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.54f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f); // 折线红色
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f); // 柱状图红色
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 0.00f, 0.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.00f, 0.00f, 0.35f);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void EditorUI::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // #ifdef IMGUI_HAS_DOCK
    //     ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    // #endif

    ShowMainMenuBar();
    if (showSceneHierarchy)
        ShowSceneHierarchy();
    if (showInspector)
        ShowInspector();
    if (showRendererSettings)
        ShowRendererSettings();
    if (showMaterialEditor)
        ShowMaterialEditor();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorUI::ShowMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(ConvertToUTF8(L"文件").c_str()))
        {
            if (ImGui::MenuItem(ConvertToUTF8(L"新建场景").c_str()))
            {
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"打开场景").c_str()))
            {
            }
            if (ImGui::MenuItem(ConvertToUTF8(L"保存场景").c_str()))
            {
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ConvertToUTF8(L"视图").c_str()))
        {
            ImGui::MenuItem(ConvertToUTF8(L"场景层级").c_str(), NULL, &showSceneHierarchy);
            ImGui::MenuItem(ConvertToUTF8(L"检视器").c_str(), NULL, &showInspector);
            ImGui::MenuItem(ConvertToUTF8(L"渲染器设置").c_str(), NULL, &showRendererSettings);
            ImGui::MenuItem(ConvertToUTF8(L"材质编辑器").c_str(), NULL, &showMaterialEditor);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void EditorUI::ShowModelCreationDialog()
{
    // ImGui::OpenPopup("Create Model");
    if (ImGui::BeginPopup(ConvertToUTF8(L"导入模型").c_str()))
    {
        // 用FileDialog选择模型文件
        ImGui::Text("%s", ConvertToUTF8(L"选择模型文件:").c_str());

        if (ImGui::Button(ConvertToUTF8(L"浏览").c_str()))
        {
            std::string modelPath =
                FileDialog::OpenFile(ConvertToUTF8(L"选择模型文件").c_str(), "Model Files\0*.obj;*.fbx;*.gltf;*.glb\0");
            if (!modelPath.empty())
            {
                // 这里可以处理模型路径
                renderer->LoadModel(modelPath);
                ImGui::CloseCurrentPopup(); // 关闭对话框
            }
        }
        ImGui::EndPopup();
    }
}

void EditorUI::ShowPrimitiveSelectionDialog()
{
    // ImGui::OpenPopup("Create Primitive");
    if (ImGui::BeginPopup(ConvertToUTF8(L"创建简单几何体").c_str()))
    {
        ImGui::Text("%s", ConvertToUTF8(L"几何体类型：").c_str());
        static int primitiveType = 0;
        ImGui::RadioButton(ConvertToUTF8(L"球").c_str(), &primitiveType, 0);
        ImGui::RadioButton(ConvertToUTF8(L"立方体").c_str(), &primitiveType, 1);
        ImGui::RadioButton(ConvertToUTF8(L"圆柱体").c_str(), &primitiveType, 2);
        ImGui::RadioButton(ConvertToUTF8(L"圆锥体").c_str(), &primitiveType, 3);
        ImGui::RadioButton(ConvertToUTF8(L"棱柱").c_str(), &primitiveType, 4);
        ImGui::RadioButton(ConvertToUTF8(L"金字塔").c_str(), &primitiveType, 5);
        ImGui::RadioButton(ConvertToUTF8(L"环面").c_str(), &primitiveType, 6);
        ImGui::RadioButton(ConvertToUTF8(L"椭球体").c_str(), &primitiveType, 7);
        ImGui::RadioButton(ConvertToUTF8(L"圆台").c_str(), &primitiveType, 8);
        ImGui::RadioButton(ConvertToUTF8(L"箭头").c_str(), &primitiveType, 9);

        if (ImGui::Button(ConvertToUTF8(L"创建").c_str()))
        {
            // 创建几何体逻辑
            renderer->CreatePrimitive(static_cast<Geometry::Type>(primitiveType), glm::vec3(0.0f), glm::vec3(1.0f),
                                      glm::vec3(0.0f), Material());
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
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
            renderer->AddLight(light);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EditorUI::ShowSceneHierarchy()
{
    // 能紧贴在应用程序的上下左右和悬空 能够被拖拽移动 窗口
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);      // 设置窗口位置
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver); // 设置窗口大小
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);         // 设置窗口视口
    ImGui::SetNextWindowBgAlpha(0.5f);                                  // 设置背景透明
    ImGui::Begin(ConvertToUTF8(L"场景层级").c_str());
    if (ImGui::TreeNode(ConvertToUTF8(L"模型").c_str()))
    {
        for (int i = 0; i < renderer->GetModelCount(); ++i)
        {
            if (ImGui::Selectable(renderer->GetModel(i)->GetName().c_str(), selectedObjectIndex == i))
            {
                selectedObjectIndex = i;
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode(ConvertToUTF8(L"简单几何体").c_str()))
    {
        auto &primitives = renderer->GetPrimitives();
        for (int i = 0; i < primitives.size(); ++i)
        {
            std::string name = ConvertToUTF8(L"几何体 ") + std::to_string(i);
            if (ImGui::Selectable(name.c_str(), selectedObjectIndex == i + renderer->GetModelCount()))
            {
                selectedObjectIndex = i + renderer->GetModelCount();
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode(ConvertToUTF8(L"光源").c_str()))
    {
        const auto &lights = renderer->GetLights();
        auto &primitives = renderer->GetPrimitives();
        for (int i = 0; i < lights.size(); ++i)
        {
            std::string name = ConvertToUTF8(L"光源 ") + std::to_string(i);
            if (ImGui::Selectable(name.c_str(),
                                  selectedObjectIndex == i + renderer->GetModelCount() + primitives.size()))
            {
                selectedObjectIndex = i + renderer->GetModelCount() + primitives.size();
            }
        }
        ImGui::TreePop();
    }
    static bool showModelDialog = false;
    static bool showPrimitiveDialog = false;
    static bool showLightDialog = false;
    if (ImGui::Button(ConvertToUTF8(L"导入模型").c_str()))
    {
        showModelDialog = true;
    }
    if (ImGui::Button(ConvertToUTF8(L"创建简单几何体").c_str()))
    {
        showPrimitiveDialog = true;
    }
    if (ImGui::Button(ConvertToUTF8(L"创建光源").c_str()))
    {
        showLightDialog = true;
    }

    // 在同一个函数内处理弹出窗口
    if (showModelDialog)
    {
        ImGui::OpenPopup(ConvertToUTF8(L"导入模型").c_str());
        showModelDialog = false;
    }
    if (showPrimitiveDialog)
    {
        ImGui::OpenPopup(ConvertToUTF8(L"创建简单几何体").c_str());
        showPrimitiveDialog = false;
    }
    if (showLightDialog)
    {
        ImGui::OpenPopup(ConvertToUTF8(L"创建光源").c_str());
        showLightDialog = false;
    }

    // 显示对话框（这些会自己处理BeginPopup/EndPopup）
    ShowModelCreationDialog();
    ShowPrimitiveSelectionDialog();
    ShowLightSelectionDialog();

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
    if (ImGui::Combo(ConvertToUTF8(L"材质类型").c_str(), &materialType, ConvertToUTF8(L"Blinn-Phong\0PBR\0").c_str()))
    {
        // 类型改变时的处理
        material.type = static_cast<MaterialType>(materialType);
    }

    if (material.type == BLINN_PHONG)
    {
        ImGui::ColorEdit3(ConvertToUTF8(L"环境光").c_str(), glm::value_ptr(material.ambient));
        ImGui::ColorEdit3(ConvertToUTF8(L"漫反射").c_str(), glm::value_ptr(material.diffuse));
        ImGui::ColorEdit3(ConvertToUTF8(L"高光").c_str(), glm::value_ptr(material.specular));
        ImGui::DragFloat(ConvertToUTF8(L"高光系数").c_str(), &material.shininess, 1.0f, 1.0f, 256.0f);

        // 贴图选择器
        TextureSelector(ConvertToUTF8(L"环境光贴图").c_str(), material.ambientMap, "ambient");
        TextureSelector(ConvertToUTF8(L"漫反射贴图").c_str(), material.diffuseMap, "diffuse");
        TextureSelector(ConvertToUTF8(L"高光贴图").c_str(), material.specularMap, "specular");
        TextureSelector(ConvertToUTF8(L"法线贴图").c_str(), material.normalMap, "normal");

        material.useAmbientMap = material.ambientMap != nullptr;
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
    ImGui::Text("%s", label.c_str());

    if (texture)
    {
        ImGui::Text("%s: %s", ConvertToUTF8(L"当前").c_str(), std::to_string(texture->GetID()).c_str());
    }
    else
    {
        ImGui::Text("%s: None", ConvertToUTF8(L"当前").c_str());
    }

    // 使用唯一ID创建按钮
    std::string buttonLabel = ConvertToUTF8(L"选择").c_str();
    buttonLabel += "##" + label + idSuffix;
    if (ImGui::Button(buttonLabel.c_str()))
    {
        std::string path = FileDialog::OpenFile(ConvertToUTF8(L"选择贴图").c_str(),
                                                "Image Files\0*.jpg;*.png;*.tga;*.bmp\0All Files\0*.*\0");
        if (!path.empty())
        {
            texture = std::make_shared<Texture>();
            if (texture->LoadFromFile(path))
            {
            }
            else
            {
                texture.reset();
                ImGui::Text("%s: %s", ConvertToUTF8(L"贴图加载失败").c_str(), path.c_str());
            }
        }
    }

    // 添加清除按钮
    std::string clearLabel = ConvertToUTF8(L"清除").c_str();
    clearLabel += "##" + label + idSuffix;
    if (ImGui::Button(clearLabel.c_str()))
    {
        texture.reset();
    }

    ImGui::Separator();
}

void EditorUI::ShowRendererSettings()
{
    ImGui::Begin(ConvertToUTF8(L"渲染器设置").c_str());

    // 渲染模式
    static const char *renderModes[] = {"Forward", "Deferred"};
    int currentMode = static_cast<int>(renderer->GetRenderMode());
    if (ImGui::Combo(ConvertToUTF8(L"渲染模式").c_str(), &currentMode, renderModes, IM_ARRAYSIZE(renderModes)))
    {
        renderer->SetRenderMode(static_cast<Renderer::RenderMode>(currentMode));
    }

    // 特效开关
    bool gamma = renderer->IsGammaCorrectionEnabled();
    if (ImGui::Checkbox(ConvertToUTF8(L"伽马校正").c_str(), &gamma))
    {
        renderer->SetGammaCorrection(gamma);
    }

    bool msaa = renderer->IsMSAAEnabled();
    if (ImGui::Checkbox("MSAA", &msaa))
    {
        renderer->SetMSAA(msaa);
    }

    bool hdr = renderer->IsHDREnabled();
    if (ImGui::Checkbox("HDR", &hdr))
    {
        renderer->SetHDR(hdr);
    }

    bool bloom = renderer->IsBloomEnabled();
    if (ImGui::Checkbox(ConvertToUTF8(L"泛光").c_str(), &bloom))
    {
        renderer->SetBloom(bloom);
    }

    bool ssao = renderer->IsSSAOEnabled();
    if (ImGui::Checkbox("SSAO", &ssao))
    {
        renderer->SetSSAO(ssao);
    }

    bool shadow = renderer->IsShadowEnabled();
    if (ImGui::Checkbox(ConvertToUTF8(L"阴影").c_str(), &shadow))
    {
        renderer->SetShadow(shadow);
    }

    bool pbr = renderer->IsPBREnabled();
    if (ImGui::Checkbox("PBR", &pbr))
    {
        renderer->SetPBR(pbr);
    }

    bool ibl = renderer->IsIBLEnabled();
    if (ImGui::Checkbox("IBL", &ibl))
    {
        renderer->SetIBL(ibl);
    }

    bool showLights = renderer->IsLightsEnabled();
    if (ImGui::Checkbox(ConvertToUTF8(L"显示光源").c_str(), &showLights))
    {
        renderer->SetLightsEnabled(showLights);
    }
    ImGui::End();
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
    }
    else if (light.getType() == 2)
    {
        auto &spotLight = static_cast<SpotLight &>(light);
        ImGui::Text("%s", ConvertToUTF8(L"聚光灯").c_str());
        ImGui::DragFloat3(ConvertToUTF8(L"位置").c_str(), glm::value_ptr(spotLight.position), 0.1f);
        ImGui::DragFloat3(ConvertToUTF8(L"方向").c_str(), glm::value_ptr(spotLight.direction), 0.01f, -1.0f, 1.0f);
        //spotLight.direction = glm::normalize(spotLight.direction);
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
    }
}