#include "ui/EditorUI.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void EditorUI::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    #ifdef IMGUI_HAS_DOCK
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    #endif

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
        auto &lights = renderer->GetLights();
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

            // 材质编辑器
            for (auto &mesh : model->GetMeshes())
            {
                if (ImGui::TreeNode(mesh->GetName().c_str()))
                {
                    ShowMaterialEditor(*mesh->GetMaterial());
                    ImGui::TreePop();
                }
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

            // 材质编辑器
            ShowMaterialEditor(*primitive.material);
        }
        else
        {
            // 显示光源属性
            auto light = renderer->GetLights()[selectedObjectIndex - modelCount - primitiveCount];
            light->OnInspectorGUI();
        }
    }

    ImGui::End();
}

void EditorUI::ShowMaterialEditor(Material &material)
{
    if (ImGui::Combo("Material Type", (int *)&material.type, "Blinn-Phong\0PBR\0"))
    {
        // 类型改变时的处理
    }

    if (material.type == BLINN_PHONG)
    {
        ImGui::ColorEdit3("Ambient", glm::value_ptr(material.ambient));
        ImGui::ColorEdit3("Diffuse", glm::value_ptr(material.diffuse));
        ImGui::ColorEdit3("Specular", glm::value_ptr(material.specular));
        ImGui::DragFloat("Shininess", &material.shininess, 1.0f, 1.0f, 256.0f);

        // 贴图选择器
        TextureSelector("Ambient Map", material.ambientMap);
        TextureSelector("Diffuse Map", material.diffuseMap);
        TextureSelector("Specular Map", material.specularMap);
        TextureSelector("Normal Map", material.normalMap);
    }
    else
    {
        ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
        ImGui::DragFloat("Metallic", &material.metallic, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Roughness", &material.roughness, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("AO", &material.ao, 0.01f, 0.0f, 1.0f);

        // 贴图选择器
        TextureSelector("Albedo Map", material.albedoMap);
        TextureSelector("Metallic Map", material.metallicMap);
        TextureSelector("Roughness Map", material.roughnessMap);
        TextureSelector("AO Map", material.aoMap);
        TextureSelector("Normal Map", material.normalMap);
    }
}

void EditorUI::ShowMaterialEditor()
{
    ImGui::Begin("Material Editor");
    ImGui::End();
}

void EditorUI::TextureSelector(const char *label, std::shared_ptr<Texture> &texture)
{
    ImGui::Text("%s", label);
    ImGui::SameLine();

    if (texture)
    {
        ImGui::Text("%s", texture->GetPath().c_str());
        ImGui::SameLine();

        if (ImGui::Button("Clear"))
        {
            texture.reset();
        }
    }
    else
    {
        if (ImGui::Button("Select"))
        {
            // 打开文件对话框选择纹理
        }
    }
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

    ImGui::End();
}

void EditorUI::EndFrame()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}