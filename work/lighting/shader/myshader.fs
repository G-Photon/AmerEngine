#version 330 core
#define MAX_LIGHTS 16
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    int useDiffuseTexture;
    int useSpecularTexture;
};

struct Light {
    int type;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    // 定向光
    vec3 direction;
    
    // 点光源
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    
    // 聚光灯
    float cutOff;
    float outerCutOff;
};

uniform int numLights;
uniform Light lights[MAX_LIGHTS];
uniform vec3 viewPos;
uniform Material material;


vec3 CalculateLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir;
    float attenuation = 1.0;

    vec3 diffuseColor = material.useDiffuseTexture == 1 ? 
        texture(material.diffuse, TexCoords).rgb : 
        material.diffuseColor;
    
    vec3 specularColor = material.useSpecularTexture == 1 ? 
        texture(material.specular, TexCoords).rgb : 
        material.specularColor;

    if (light.type == 0) { // 定向光
        lightDir = normalize(-light.direction);
    } else { // 点光源/聚光灯
        lightDir = normalize(light.position - fragPos);
        float distance = length(light.position - fragPos);
        attenuation = 1.0 / (light.constant + light.linear * distance + 
                      light.quadratic * (distance * distance));
    }
    
    // 聚光灯计算
    float intensity = 1.0;
    if (light.type == 2) {
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    }
    
    // 光照计算...
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseColor;

    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularColor;
    // 环境光
    vec3 ambient = light.ambient * diffuseColor;
    // 合并结果
    vec3 finalColor = ambient + diffuse + specular;
    // 衰减
    if (light.type == 1 || light.type == 2) {
        finalColor *= attenuation * intensity;
    }

    return finalColor;
}

void main() {
    vec3 result = vec3(0.0);
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    for(int i = 0; i < numLights; i++) {
        result += CalculateLight(lights[i], norm, FragPos, viewDir);
    }
    FragColor = vec4(result, 1.0);
}