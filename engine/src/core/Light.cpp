#include "core/Light.hpp"

int PointLight::count = 0;
int DirectionalLight::count = 0;
int SpotLight::count = 0;
// 点光源
PointLight::PointLight(const glm::vec3 &position, const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular, float intensity)
    : position(position), ambient(ambient), diffuse(diffuse), specular(specular), intensity(intensity), constant(1.0f), linear(0.09f), quadratic(0.032f)
{
    number = count++;
}

void PointLight::SetupShader(Shader &shader, int index) const
{
    std::string prefix = "pointLights[" + std::to_string(index) + "].";
    shader.SetVec3(prefix + "position", position);
    shader.SetVec3(prefix + "ambient", ambient * intensity);
    shader.SetVec3(prefix + "diffuse", diffuse * intensity);
    shader.SetVec3(prefix + "specular", specular * intensity);
    shader.SetFloat(prefix + "constant", constant);
    shader.SetFloat(prefix + "linear", linear);
    shader.SetFloat(prefix + "quadratic", quadratic);
}

// 方向光
DirectionalLight::DirectionalLight(const glm::vec3 &direction, const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular, float intensity)
    : direction(glm::normalize(direction)), ambient(ambient), diffuse(diffuse), specular(specular), intensity(intensity)
{
    number = count++;
}

void DirectionalLight::SetupShader(Shader &shader, int index) const
{
    std::string prefix = "dirLight[" + std::to_string(index) + "].";
    shader.SetVec3(prefix + "direction", direction);
    shader.SetVec3(prefix + "ambient", ambient * intensity);
    shader.SetVec3(prefix + "diffuse", diffuse * intensity);
    shader.SetVec3(prefix + "specular", specular * intensity);
}

// 聚光灯
SpotLight::SpotLight(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular, float intensity,
                     float cutOff, float outerCutOff)
    : position(position), direction(glm::normalize(direction)), ambient(ambient), diffuse(diffuse), specular(specular), intensity(intensity),
      cutOff(glm::cos(glm::radians(cutOff))), outerCutOff(glm::cos(glm::radians(outerCutOff))), constant(1.0f),
      linear(0.09f), quadratic(0.032f)
{
    number = count++;
}

void SpotLight::SetupShader(Shader &shader, int index) const
{
    std::string prefix = "spotLights[" + std::to_string(index) + "].";
    shader.SetVec3(prefix + "position", position);
    shader.SetVec3(prefix + "direction", direction);
    shader.SetVec3(prefix + "ambient", ambient * intensity);
    shader.SetVec3(prefix + "diffuse", diffuse * intensity);
    shader.SetVec3(prefix + "specular", specular * intensity);
    shader.SetFloat(prefix + "cutOff", cutOff);
    shader.SetFloat(prefix + "outerCutOff", outerCutOff);
    shader.SetFloat(prefix + "constant", constant);
    shader.SetFloat(prefix + "linear", linear);
    shader.SetFloat(prefix + "quadratic", quadratic);
}
