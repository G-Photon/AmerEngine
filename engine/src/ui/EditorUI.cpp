#include "ui/EditorUI.hpp"
#include "utils/FileDialog.hpp"
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
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
            }
            if (ImGui::MenuItem("Open Scene"))
            {
            }
            if (ImGui::MenuItem("Save Scene"))
            {
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Scene Hierarchy", NULL, &showSceneHierarchy);
            ImGui::MenuItem("Inspector", NULL, &showInspector);
            ImGui::MenuItem("Renderer Settings", NULL, &showRendererSettings);
            ImGui::MenuItem("Material Editor", NULL, &showMaterialEditor);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void EditorUI::ShowModelCreationDialog()
{
    //ImGui::OpenPopup("Create Model");
    if (ImGui::BeginPopup("Create Model"))
    {
        //用FileDialog选择模型文件
        ImGui::Text("Select Model File:");
        if (ImGui::Button("Browse"))
        {
            std::string modelPath = FileDialog::OpenFile("Select Model File",
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
    if (ImGui::BeginPopup("Create Primitive"))
    {
        ImGui::Text("Primitive Type:");
        static int primitiveType = 0;
        ImGui::RadioButton("Sphere", &primitiveType, 0);
        ImGui::RadioButton("Cube", &primitiveType, 1);
        ImGui::RadioButton("Cylinder", &primitiveType, 2);
        ImGui::RadioButton("Cone", &primitiveType, 3);
        ImGui::RadioButton("Prism", &primitiveType, 4);
        ImGui::RadioButton("Pyramid", &primitiveType, 5);
        ImGui::RadioButton("Torus", &primitiveType, 6);
        ImGui::RadioButton("Ellipsoid", &primitiveType, 7);

        if (ImGui::Button("Create"))
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
    if (ImGui::BeginPopup("Create Light"))
    {
        ImGui::Text("Light Type:");
        static int lightType = 0;
        ImGui::RadioButton("Point Light", &lightType, 0);
        ImGui::RadioButton("Directional Light", &lightType, 1);
        ImGui::RadioButton("Spot Light", &lightType, 2);

        if (ImGui::Button("Create"))
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
    ImGui::Begin("Scene Hierarchy");

    if (ImGui::TreeNode("Models"))
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

    if (ImGui::TreeNode("Primitives"))
    {
        auto &primitives = renderer->GetPrimitives();
        for (int i = 0; i < primitives.size(); ++i)
        {
            std::string name = "Primitive " + std::to_string(i);
            if (ImGui::Selectable(name.c_str(), selectedObjectIndex == i + renderer->GetModelCount()))
            {
                selectedObjectIndex = i + renderer->GetModelCount();
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Lights"))
    {
        const auto &lights = renderer->GetLights();
        auto &primitives = renderer->GetPrimitives();
        for (int i = 0; i < lights.size(); ++i)
        {
            std::string name = "Light " + std::to_string(i);
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
    if (ImGui::Button("Create Model"))
    {
        showModelDialog = true;
    }
    if (ImGui::Button("Create Primitive"))
    {
        showPrimitiveDialog = true;
    }
    if (ImGui::Button("Create Light"))
    {
        showLightDialog = true;
    }

    // 在同一个函数内处理弹出窗口
    if (showModelDialog)
    {
        ImGui::OpenPopup("Create Model");
        showModelDialog = false;
    }
    if (showPrimitiveDialog)
    {
        ImGui::OpenPopup("Create Primitive");
        showPrimitiveDialog = false;
    }
    if (showLightDialog)
    {
        ImGui::OpenPopup("Create Light");
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
    ImGui::Begin("Inspector");
    if (selectedObjectIndex >= 0)
    {
        int modelCount = renderer->GetModelCount();
        int primitiveCount = renderer->GetPrimitives().size();

        if (selectedObjectIndex < modelCount)
        {
            // 显示模型属性
            auto model = renderer->GetModel(selectedObjectIndex);
            ImGui::Text("Model: %s", model->GetName().c_str());

            // 变换编辑器
            static glm::vec3 position, rotation, scale;
            position = model->GetPosition();
            rotation = model->GetRotation();
            scale = model->GetScale();

            ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f);
            ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 1.0f);
            ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f);

            model->SetTransform(position, rotation, scale);

            // 材质编辑器 可能有很多个不同的mesh，imgui需要分配不同id
            for (auto &mesh : model->GetMeshes())
            {
                ImGui::PushID(&mesh);
                ImGui::Text("Mesh: %s", mesh->GetName().c_str());
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
            ImGui::Text("Primitive Type: %d", primitive.type);

            // 变换编辑器
            ImGui::DragFloat3("Position", (float*) glm::value_ptr(primitive.position), 0.1f);
            ImGui::DragFloat3("Rotation", (float*) glm::value_ptr(primitive.rotation), 1.0f);
            ImGui::DragFloat3("Scale", (float*) glm::value_ptr(primitive.scale), 0.1f);

            primitive.mesh->SetTransform(primitive.position, primitive.rotation, primitive.scale);
            // 材质编辑器
            ShowMaterialEditor(*primitive.mesh->GetMaterial());
        }
        else
        {
            // 显示光源属性
            auto light = renderer->GetLights()[selectedObjectIndex - modelCount - primitiveCount];
            light->OnInspectorGUI();
        }

        // delete
        if (ImGui::Button("Delete"))
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
    if (ImGui::Combo("Material Type", &materialType, "Blinn-Phong\0PBR\0"))
    {
        // 类型改变时的处理
        material.type = static_cast<MaterialType>(materialType);
    }

    if (material.type == BLINN_PHONG)
    {
        ImGui::ColorEdit3("Ambient", glm::value_ptr(material.ambient));
        ImGui::ColorEdit3("Diffuse", glm::value_ptr(material.diffuse));
        ImGui::ColorEdit3("Specular", glm::value_ptr(material.specular));
        ImGui::DragFloat("Shininess", &material.shininess, 1.0f, 1.0f, 256.0f);

        // 贴图选择器
        TextureSelector("Ambient Map", material.ambientMap, "ambient");
        TextureSelector("Diffuse Map", material.diffuseMap, "diffuse");
        TextureSelector("Specular Map", material.specularMap, "specular");
        TextureSelector("Normal Map", material.normalMap, "normal");

        material.useAmbientMap = material.ambientMap != nullptr;
        material.useDiffuseMap = material.diffuseMap != nullptr;
        material.useSpecularMap = material.specularMap != nullptr;
        material.useNormalMap = material.normalMap != nullptr;
    }
    else
    {
        ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
        ImGui::DragFloat("Metallic", &material.metallic, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Roughness", &material.roughness, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("AO", &material.ao, 0.01f, 0.0f, 1.0f);

        // 贴图选择器
        TextureSelector("Albedo Map", material.albedoMap, "albedo");
        TextureSelector("Metallic Map", material.metallicMap, "metallic");
        TextureSelector("Roughness Map", material.roughnessMap, "roughness");
        TextureSelector("AO Map", material.aoMap, "ao");
        TextureSelector("Normal Map", material.normalMap, "normal");
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
    ImGui::Begin("Material Editor");

    // Example: If you want to show all materials from the renderer
    const auto &allMaterials = renderer->getALLMaterials();
    for (auto &material : allMaterials)
    {
        ImGui::PushID(std::hash<std::shared_ptr<Material>>{}(material));
        ImGui::Text("Material: %s", material->name.c_str());
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
        ImGui::Text("Current: %s", std::to_string(texture->GetID()).c_str());
    }
    else
    {
        ImGui::Text("Current: None");
    }

    // 使用唯一ID创建按钮
    std::string buttonLabel = "Select##" + label + idSuffix;
    if (ImGui::Button(buttonLabel.c_str()))
    {
        std::string path =
            FileDialog::OpenFile("Select Texture", "Image Files\0*.jpg;*.png;*.tga;*.bmp\0All Files\0*.*\0");
        if (!path.empty())
        {
            texture = std::make_shared<Texture>();
            if (texture->LoadFromFile(path))
            {
            }
            else
            {
                texture.reset();
                ImGui::Text("Failed to load texture: %s", path.c_str());
            }
        }
    }

    // 添加清除按钮
    std::string clearLabel = "Clear##" + label + idSuffix;
    if (ImGui::Button(clearLabel.c_str()))
    {
        texture.reset();
    }

    ImGui::Separator();
}

void EditorUI::ShowRendererSettings()
{
    ImGui::Begin("Renderer Settings");

    // 渲染模式
    static const char *renderModes[] = {"Forward", "Deferred"};
    int currentMode = static_cast<int>(renderer->GetRenderMode());
    if (ImGui::Combo("Render Mode", &currentMode, renderModes, IM_ARRAYSIZE(renderModes)))
    {
        renderer->SetRenderMode(static_cast<Renderer::RenderMode>(currentMode));
    }

    // 特效开关
    bool gamma = renderer->IsGammaCorrectionEnabled();
    if (ImGui::Checkbox("Gamma Correction", &gamma))
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
    if (ImGui::Checkbox("Bloom", &bloom))
    {
        renderer->SetBloom(bloom);
    }

    bool ssao = renderer->IsSSAOEnabled();
    if (ImGui::Checkbox("SSAO", &ssao))
    {
        renderer->SetSSAO(ssao);
    }

    bool shadow = renderer->IsShadowEnabled();
    if (ImGui::Checkbox("Shadows", &shadow))
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
    if (ImGui::Checkbox("Show Lights", &showLights))
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