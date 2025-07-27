#include "core/Light.hpp"
#include "glm/trigonometric.hpp"

static void RenderQuad()
{
    static GLuint quadVAO = 0, quadVBO = 0;
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

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

void PointLight::drawLightMesh(const std::unique_ptr<Shader> &shader)
{
    // 1. 复用单位球体（可缓存）
    static auto lightMesh = Geometry::CreateSphere(1.0f, 32);

    // 2. 根据衰减系数计算光体积半径
    //    经验公式：亮度降到 1/256 时的距离
    float maxBrightness = std::max({diffuse.r, diffuse.g, diffuse.b}) * intensity;
    float denom = 256.0f * maxBrightness;
    float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - denom))) / (2.0f * quadratic);
    radius = std::max(radius, 0.01f);

    // 3. 构建模型矩阵
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(radius));

    // 4. 把矩阵和光源参数送入 shader
    shader->SetMat4("model", model);
    shader->SetVec3("light.position", position);
    shader->SetVec3("light.ambient", ambient * intensity);
    shader->SetVec3("light.diffuse", diffuse * intensity);
    shader->SetVec3("light.specular", specular * intensity);
    shader->SetFloat("light.constant", constant);
    shader->SetFloat("light.linear", linear);
    shader->SetFloat("light.quadratic", quadratic);
    shader->SetInt("lightType", getType());
    // 5. 绘制球体
    lightMesh->Draw(*shader);
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

void DirectionalLight::drawLightMesh(const std::unique_ptr<Shader> &shader)
{
    /*-------------------------------------------------
     * 1. 方向光没有位置，只有方向；无衰减
     *------------------------------------------------*/
    shader->SetMat4("model", glm::mat4(1.0f)); // 单位矩阵
    shader->SetVec3("light.direction", glm::normalize(direction));
    shader->SetVec3("light.ambient", ambient * intensity);
    shader->SetVec3("light.diffuse", diffuse * intensity);
    shader->SetVec3("light.specular", specular * intensity);
    shader->SetInt("lightType", getType());

    /*-------------------------------------------------
     * 2. 直接绘制全屏 Quad
     *------------------------------------------------*/
    // Ensure RenderQuad is declared somewhere accessible, or define it here if missing.
    // Example stub implementation (replace with actual implementation as needed):
    RenderQuad(); // 复用 Renderer 的全屏三角带
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

void SpotLight::drawLightMesh(const std::unique_ptr<Shader> &shader)
{
    static auto lightCone = Geometry::CreateCone(1.0f, 1.0f, 32);

    /* 1. 根据衰减求有效照射距离（替代缺失的 range） */
    float maxChannel = glm::max(glm::max(diffuse.r, diffuse.g), diffuse.b) * intensity;
    float threshold = maxChannel / 256.0f;
    float discriminant = linear * linear - 4.0f * quadratic * (constant - 1.0f / threshold);
    float range = 0.0f;
    if (discriminant >= 0.0f)
        range = (-linear + glm::sqrt(discriminant)) / (2.0f * quadratic);
    range = glm::max(range, 0.01f);

    /* 2. 计算光锥底面半径 */
    float outerAngleRad = glm::radians(outerCutOff);
    float baseRadius = range * tan(outerAngleRad);

    /* 3. 模型矩阵：缩放 → 旋转 → 平移 */
    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(baseRadius, range, baseRadius));

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 dirNorm = glm::normalize(direction);
    glm::vec3 right = glm::normalize(glm::cross(up, dirNorm));
    glm::vec3 newUp = glm::cross(dirNorm, right);
    glm::mat4 rot(1.0f);
    rot[0] = glm::vec4(right, 0.0f);
    rot[1] = glm::vec4(newUp, 0.0f);
    rot[2] = glm::vec4(dirNorm, 0.0f);
    model = glm::translate(glm::mat4(1.0f), position) * rot * model;

    /* 4. 传 uniform（与旧 SetupShader 字段保持一致） */
    shader->SetMat4("model", model);
    shader->SetVec3("light.position", position);
    shader->SetVec3("light.direction", direction);
    shader->SetVec3("light.ambient", ambient * intensity);
    shader->SetVec3("light.diffuse", diffuse * intensity);
    shader->SetVec3("light.specular", specular * intensity);
    shader->SetFloat("light.cutOff", cutOff);
    shader->SetFloat("light.outerCutOff", outerCutOff);
    shader->SetFloat("light.constant", constant);
    shader->SetFloat("light.linear", linear);
    shader->SetFloat("light.quadratic", quadratic);
    shader->SetInt("lightType", getType());

    /* 5. 绘制 */
    lightCone->Draw(*shader);
}