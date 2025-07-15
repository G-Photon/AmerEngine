#include "core/Light.hpp"

// 点光源
PointLight::PointLight(const glm::vec3 &position, const glm::vec3 &color, float intensity)
    : position(position), color(color), intensity(intensity), constant(1.0f), linear(0.09f), quadratic(0.032f)
{
}

void PointLight::SetupShader(Shader &shader, int index) const
{
    std::string prefix = "pointLights[" + std::to_string(index) + "].";
    shader.SetVec3(prefix + "position", position);
    shader.SetVec3(prefix + "color", color * intensity);
    shader.SetFloat(prefix + "constant", constant);
    shader.SetFloat(prefix + "linear", linear);
    shader.SetFloat(prefix + "quadratic", quadratic);
}

void PointLight::OnInspectorGUI()
{
    ImGui::Text("Point Light");
    ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f);
    ImGui::ColorEdit3("Color", glm::value_ptr(color));
    ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Constant", &constant, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Linear", &linear, 0.001f, 0.0f, 1.0f);
    ImGui::DragFloat("Quadratic", &quadratic, 0.0001f, 0.0f, 1.0f);
}

// 方向光
DirectionalLight::DirectionalLight(const glm::vec3 &direction, const glm::vec3 &color, float intensity)
    : direction(glm::normalize(direction)), color(color), intensity(intensity)
{
}

void DirectionalLight::SetupShader(Shader &shader) const
{
    shader.SetVec3("dirLight.direction", direction);
    shader.SetVec3("dirLight.color", color * intensity);
}

void DirectionalLight::OnInspectorGUI()
{
    ImGui::Text("Directional Light");
    ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.01f);
    direction = glm::normalize(direction);
    ImGui::ColorEdit3("Color", glm::value_ptr(color));
    ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f);
}

// 聚光灯
SpotLight::SpotLight(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &color, float intensity,
                     float cutOff, float outerCutOff)
    : position(position), direction(glm::normalize(direction)), color(color), intensity(intensity),
      cutOff(glm::cos(glm::radians(cutOff))), outerCutOff(glm::cos(glm::radians(outerCutOff))), constant(1.0f),
      linear(0.09f), quadratic(0.032f)
{
}

void SpotLight::SetupShader(Shader &shader, int index) const
{
    std::string prefix = "spotLights[" + std::to_string(index) + "].";
    shader.SetVec3(prefix + "position", position);
    shader.SetVec3(prefix + "direction", direction);
    shader.SetVec3(prefix + "color", color * intensity);
    shader.SetFloat(prefix + "cutOff", cutOff);
    shader.SetFloat(prefix + "outerCutOff", outerCutOff);
    shader.SetFloat(prefix + "constant", constant);
    shader.SetFloat(prefix + "linear", linear);
    shader.SetFloat(prefix + "quadratic", quadratic);
}

void SpotLight::OnInspectorGUI()
{
    ImGui::Text("Spot Light");
    ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f);
    ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.01f);
    direction = glm::normalize(direction);
    ImGui::ColorEdit3("Color", glm::value_ptr(color));
    ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f);

    float degrees = glm::degrees(glm::acos(cutOff));
    if (ImGui::DragFloat("Cut Off", &degrees, 1.0f, 0.0f, 90.0f))
    {
        cutOff = glm::cos(glm::radians(degrees));
    }

    degrees = glm::degrees(glm::acos(outerCutOff));
    if (ImGui::DragFloat("Outer Cut Off", &degrees, 1.0f, 0.0f, 90.0f))
    {
        outerCutOff = glm::cos(glm::radians(degrees));
    }

    ImGui::DragFloat("Constant", &constant, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Linear", &linear, 0.001f, 0.0f, 1.0f);
    ImGui::DragFloat("Quadratic", &quadratic, 0.0001f, 0.0f, 1.0f);
}