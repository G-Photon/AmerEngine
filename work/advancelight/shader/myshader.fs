#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
#define MAX_LIGHTS 16
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;
uniform samplerCube skybox;
uniform sampler2D texture_height1;
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    //sampler2D reflect;
    vec4 diffuseColor;
    vec4 specularColor;
    float shininess;
    int useDiffuseTexture;
    int useSpecularTexture;
    int useReflectTexture;
    int useNormalTexture;
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


vec4 CalculateLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir;
    float attenuation = 1.0;

    vec4 diffuseColor = material.useDiffuseTexture == 1 ? 
        texture(material.diffuse, TexCoords).rgba : 
        material.diffuseColor;

    vec4 specularColor = material.useSpecularTexture == 1 ? 
        texture(material.specular, TexCoords).rgba : 
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
    vec4 diffuse = vec4(light.diffuse, 1.0) * diff * diffuseColor;

    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec4 specular = vec4(light.specular, 1.0) * spec * specularColor;
    // 环境光
    vec4 ambient = vec4(light.ambient, 1.0) * diffuseColor;
    // 如果使用反射贴图
    if (material.useReflectTexture == 1) {
        ambient += vec4(vec3(texture(skybox, reflect(normalize(-viewDir), normal))) * texture(texture_height1, TexCoords).rgb, 1.0);
    }
    // 合并结果
    vec4 finalColor = ambient + diffuse + specular;
    // 衰减
    if (light.type == 1 || light.type == 2) {
        finalColor *= attenuation * intensity;
    }

    return finalColor;
}

void main() {
    vec4 result = vec4(0.0);
    vec3 norm;
    if (material.useNormalTexture == 1) {
        // 使用法线贴图
        norm = normalize(TBN * (texture(material.normal, TexCoords).rgb * 2.0 - 1.0));
    } else {
        // 使用顶点法线
        norm = normalize(Normal);
    }
    vec3 viewDir = normalize(viewPos - FragPos);

    for(int i = 0; i < numLights; i++) {
        result += CalculateLight(lights[i], norm, FragPos, viewDir);
    }
    FragColor = result;
    float brightness = dot(result.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 5.0) {
        BrightColor = vec4(result.rgb, 1.0);
    } else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}