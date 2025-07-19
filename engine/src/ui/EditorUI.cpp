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
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    ImGui::StyleColorsDark();

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
    //ImGui::OpenPopup("Create Model");
    if (ImGui::BeginPopup(ConvertToUTF8(L"导入模型").c_str()))
    {
        //用FileDialog选择模型文件
        ImGui::Text("%s", ConvertToUTF8(L"选择模型文件:").c_str());
        
        if (ImGui::Button(ConvertToUTF8(L"浏览").c_str()))
        {
            std::string modelPath = FileDialog::OpenFile(ConvertToUTF8(L"选择模型文件").c_str(),
                                                         "Model Files\0*.obj;*.fbx;*.gltf;*.glb\0");
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
    //ImGui::OpenPopup("Create Primitive");
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
        ImGui::RadioButton(ConvertToUTF8(L"截头锥").c_str(), &primitiveType, 8);

        if (ImGui::Button(ConvertToUTF8(L"创建").c_str()))
        {
            // 创建几何体逻辑
            renderer->CreatePrimitive(static_cast<Geometry::Type>(primitiveType),
                                      glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f),
                                      Material());
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EditorUI::ShowLightSelectionDialog()
{
    //ImGui::OpenPopup("Create Light");
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
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver); // 设置窗口位置
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver); // 设置窗口大小
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID); // 设置窗口视口
    ImGui::SetNextWindowBgAlpha(0.5f); // 设置背景透明
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
            ImGui::DragFloat3(ConvertToUTF8(L"位置").c_str(), (float*) glm::value_ptr(primitive.position), 0.1f);
            ImGui::DragFloat3(ConvertToUTF8(L"旋转").c_str(), (float*) glm::value_ptr(primitive.rotation), 1.0f);
            ImGui::DragFloat3(ConvertToUTF8(L"缩放").c_str(), (float*) glm::value_ptr(primitive.scale), 0.1f);

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
        std::string path =
            FileDialog::OpenFile(ConvertToUTF8(L"选择贴图").c_str(), "Image Files\0*.jpg;*.png;*.tga;*.bmp\0All Files\0*.*\0");
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
        ImGui::DragFloat3(ConvertToUTF8(L"方向").c_str(), glm::value_ptr(directionalLight.direction), 0.1f);
        directionalLight.direction = glm::normalize(directionalLight.direction);
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
        ImGui::DragFloat3(ConvertToUTF8(L"方向").c_str(), glm::value_ptr(spotLight.direction), 0.01f);
        spotLight.direction = glm::normalize(spotLight.direction);
        ImGui::ColorEdit3(ConvertToUTF8(L"环境光").c_str(), glm::value_ptr(spotLight.ambient));
        ImGui::ColorEdit3(ConvertToUTF8(L"漫反射").c_str(), glm::value_ptr(spotLight.diffuse));
        ImGui::ColorEdit3(ConvertToUTF8(L"高光").c_str(), glm::value_ptr(spotLight.specular));
        ImGui::DragFloat(ConvertToUTF8(L"强度").c_str(), &spotLight.intensity, 0.1f, 0.0f, 100.0f);

        float degrees = glm::degrees(glm::acos(spotLight.cutOff));
        if (ImGui::DragFloat(ConvertToUTF8(L"内切角").c_str(), &degrees, 1.0f, 0.0f, 90.0f))
        {
            spotLight.cutOff = glm::cos(glm::radians(degrees));
        }

        degrees = glm::degrees(glm::acos(spotLight.outerCutOff));
        if (ImGui::DragFloat(ConvertToUTF8(L"外切角").c_str(), &degrees, 1.0f, 0.0f, 90.0f))
        {
            spotLight.outerCutOff = glm::cos(glm::radians(degrees));
        }

        ImGui::DragFloat(ConvertToUTF8(L"常数项").c_str(), &spotLight.constant, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat(ConvertToUTF8(L"线性项").c_str(), &spotLight.linear, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat(ConvertToUTF8(L"二次项").c_str(), &spotLight.quadratic, 0.0001f, 0.0f, 1.0f);
    }
}