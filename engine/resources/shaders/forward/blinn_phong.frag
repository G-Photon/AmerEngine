#version 460 core

struct Material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
    
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D normalMap;
    
    bool useDiffuseMap;
    bool useSpecularMap;
    bool useNormalMap;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool hasShadows;
    sampler2D shadowMap;
    mat4 lightSpaceMatrix;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    bool hasShadows;
    sampler2D shadowMap;
    mat4 lightSpaceMatrix;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform int numLights[3]; // [0]: DirLight, [1]: PointLight, [2]: SpotLight
uniform bool shadowEnabled;
uniform DirLight dirLight[NR_POINT_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_POINT_LIGHTS];
uniform bool useNormalMapping;

// 函数声明
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 fragPos);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

// 阴影计算函数
float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap);

void main() {
    // 属性
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 法线贴图
    if (material.useNormalMap && useNormalMapping) {
        // 从法线贴图获取法线 [0,1]
        vec3 normalMap = texture(material.normalMap, TexCoords).rgb;
        // 转换到 [-1,1] 范围
        normalMap = normalize(normalMap * 2.0 - 1.0);
        
        // 创建TBN矩阵
        vec3 T = normalize(Tangent);
        vec3 B = normalize(Bitangent);
        vec3 N = normalize(Normal);

        // 确保T和B正交
        T = normalize(T - dot(T, N) * N);
        B = cross(N, T);
        mat3 TBN = mat3(T, B, N);
        
        norm = normalize(TBN * normalMap);
    }
    
    // 光贡献
    vec3 result = vec3(0.0);
    //定向光贡献
    for(int i = 0; i < numLights[0]; i++) {
        result += CalcDirLight(dirLight[i], norm, viewDir, FragPos);
    }
    
    // 点光源贡献
    for(int i = 0; i < numLights[1]; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    
    // 聚光灯贡献
    for(int i = 0; i < numLights[2]; i++) {
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
    }
    
    FragColor = vec4(result, 1.0);
}

// 计算方向光
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 lightDir = normalize(-light.direction);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 镜面反射
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // 阴影计算
    float shadow = 0.0;
    if (light.hasShadows) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace, light.shadowMap);
    }
    
    // 组合结果
    vec3 ambient, diffuse, specular;

    if (material.useDiffuseMap) {
        ambient = light.ambient * vec3(texture(material.diffuseMap, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuseMap, TexCoords));
    } else {
        ambient = light.ambient * material.diffuse;
        diffuse = light.diffuse * diff * material.diffuse;
    }
    
    if (material.useSpecularMap) {
        specular = light.specular * spec * vec3(texture(material.specularMap, TexCoords));
    } else {
        specular = light.specular * spec * material.specular;
    }
    
    // 应用阴影
    diffuse *= (1.0 - shadow);
    specular *= (1.0 - shadow);
    
    return (ambient + diffuse + specular);
}

// 计算点光源
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 镜面反射
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // 阴影计算
    float shadow = 0.0;
    if (light.hasShadows) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace, light.shadowMap);
    }
    
    // 组合结果
    vec3 ambient, diffuse, specular;

    if (material.useDiffuseMap) {
        ambient = light.ambient * vec3(texture(material.diffuseMap, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuseMap, TexCoords));
    } else {
        ambient = light.ambient * material.diffuse;
        diffuse = light.diffuse * diff * material.diffuse;
    }
    
    if (material.useSpecularMap) {
        specular = light.specular * spec * vec3(texture(material.specularMap, TexCoords));
    } else {
        specular = light.specular * spec * material.specular;
    }
    
    ambient *= attenuation;
    diffuse *= attenuation * (1.0 - shadow);
    specular *= attenuation * (1.0 - shadow);
    
    return (ambient + diffuse + specular);
}

// 计算聚光灯
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 镜面反射
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // 聚光强度
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // 组合结果
    vec3 ambient, diffuse, specular;

    if (material.useDiffuseMap) {
        ambient = light.ambient * vec3(texture(material.diffuseMap, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuseMap, TexCoords));
    } else {
        ambient = light.ambient * material.diffuse;
        diffuse = light.diffuse * diff * material.diffuse;
    }
    
    if (material.useSpecularMap) {
        specular = light.specular * spec * vec3(texture(material.specularMap, TexCoords));
    } else {
        specular = light.specular * spec * material.specular;
    }
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return (ambient + diffuse + specular);
}

// 阴影计算函数
float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // 变换到[0,1]范围
    projCoords = projCoords * 0.5 + 0.5;
    
    // 检查是否在阴影贴图范围内
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    // 获取当前片段在光源视角下的深度
    float currentDepth = projCoords.z;
    
    // 检查当前片段是否在阴影中
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    // PCF软阴影
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - 0.005 > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    // 调试：如果阴影值大于0，输出一些信息
    if(shadow > 0.0) {
        // 这里可以添加调试输出
    }
    
    return shadow;
}