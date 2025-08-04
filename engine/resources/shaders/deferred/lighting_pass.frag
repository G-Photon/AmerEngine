#version 460 core
out vec4 FragColor;

// G-Buffer 纹理
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;
uniform sampler2D gMetallic;
uniform sampler2D gRoughness;
uniform sampler2D gAo;
uniform sampler2D gAmbient;
uniform sampler2D ssao;
uniform int ssaoEnabled; // 是否启用SSAO

// 光源结构体
struct L {
    vec3 position;      // 位置（点光源和聚光灯）
    vec3 direction;     // 方向（方向光和聚光灯）
    vec3 ambient;       // 环境光
    vec3 diffuse;       // 漫反射
    vec3 specular;      // 镜面反射
    
    // 衰减参数
    float constant;
    float linear;
    float quadratic;
    
    // 聚光灯参数
    float cutOff;       // 内切角余弦值
    float outerCutOff;  // 外切角余弦值
    
    // 阴影相关
    bool hasShadows;
    mat4 lightSpaceMatrix;
};
uniform L light;
uniform sampler2D lightShadowMap; // 单独定义阴影贴图

uniform int lightType; // 0:点光源, 1:方向光, 2:聚光灯
uniform vec3 viewPos;   // 相机位置
uniform vec2 screenSize; // 屏幕尺寸，用于计算纹理坐标
uniform bool shadowEnabled; // 全局阴影开关

const float PI = 3.14159265359;
const float SHININESS_FACTOR = 32.0; // 高光系数

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
    
    return shadow;
}

// 光照计算函数
vec3 calculateDirectionalLight(vec3 fragPos, vec3 normal, vec3 ambientColor, vec3 albedo, vec3 specularColor, float roughness, float ao);
vec3 calculatePointLight(vec3 fragPos, vec3 normal, vec3 ambientColor, vec3 albedo, vec3 specularColor, float roughness, float ao);
vec3 calculateSpotLight(vec3 fragPos, vec3 normal, vec3 ambientColor, vec3 albedo, vec3 specularColor, float roughness, float ao);

vec2 CalcTexCoord()
{
   return gl_FragCoord.xy / screenSize;
}

void main() {
    // 从G缓冲中获取数据
    vec2 TexCoords = CalcTexCoord();
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);

    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    vec3 specularColor = texture(gSpecular, TexCoords).rgb;
    float roughness = texture(gRoughness, TexCoords).r;
    float ao = texture(gAo, TexCoords).r;
    vec3 ambient = texture(gAmbient, TexCoords).rgb;
    float ssaoOcclusion = ssaoEnabled > 0 ? texture(ssao, TexCoords).r : 1.0;
    ao= ao * ssaoOcclusion; // 应用SSAO遮挡

    vec3 result = vec3(0.0);
    
    // 根据光源类型计算光照
    switch(lightType) {
        case 0: // 点光源
            result = calculatePointLight(fragPos, normal, ambient, albedo, specularColor, roughness, ao);
            break;
        case 1: // 方向光
            result = calculateDirectionalLight(fragPos, normal, ambient, albedo, specularColor, roughness, ao);
            break;
        case 2: // 聚光灯
            result = calculateSpotLight(fragPos, normal, ambient, albedo, specularColor, roughness, ao);
            break;
    }

    FragColor = vec4(result, 1.0);
}

// 计算方向光
vec3 calculateDirectionalLight(vec3 fragPos, vec3 normal, vec3 ambientColor, vec3 albedo, vec3 specularColor, float roughness, float ao) {
    // 光源方向（从光源指向片段）
    vec3 lightDir = normalize(-light.direction);
    // 视线方向（从片段指向相机）
    vec3 viewDir = normalize(viewPos - fragPos);
    
    // 漫反射分量
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * albedo;
    
    // 镜面反射分量（Blinn-Phong）
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), SHININESS_FACTOR);
    vec3 specular = light.specular * spec * specularColor;
    
    // 阴影计算
    float shadow = 0.0;
    if (shadowEnabled && light.hasShadows) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace, lightShadowMap);
    }
    
    // 环境光分量
    vec3 ambient = light.ambient * ambientColor * ao;
    
    // 应用阴影
    diffuse *= (1.0 - shadow);
    specular *= (1.0 - shadow);

    return ambient + diffuse + specular;
}

// 计算点光源
vec3 calculatePointLight(vec3 fragPos, vec3 normal, vec3 ambientColor, vec3 albedo, vec3 specularColor, float roughness, float ao) {
    // 光源方向（从片段指向光源）
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);
    // 视线方向（从片段指向相机）
    vec3 viewDir = normalize(viewPos - fragPos);

    // 镜面反射分量（Blinn-Phong）
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), SHININESS_FACTOR);

    // 距离衰减计算
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                             light.quadratic * (distance * distance));
    
    // 阴影计算
    float shadow = 0.0;
    if (shadowEnabled && light.hasShadows) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace, lightShadowMap);
    }
    
    // 漫反射分量
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * specularColor;
    // 环境光分量
    vec3 ambient = light.ambient * ambientColor * ao;
    
    // 应用衰减和阴影
    ambient *= attenuation;
    diffuse *= attenuation * (1.0 - shadow);
    specular *= attenuation * (1.0 - shadow);
    
    return ambient + diffuse + specular;
}

// 计算聚光灯
vec3 calculateSpotLight(vec3 fragPos, vec3 normal, vec3 ambientColor, vec3 albedo, vec3 specularColor, float roughness, float ao) {
    // 光源方向（从片段指向光源）
    vec3 lightDir = normalize(light.position - fragPos);
    // 视线方向（从片段指向相机）
    vec3 viewDir = normalize(viewPos - fragPos);
    
    // 聚光灯方向（从光源指向目标）
    vec3 spotDir = normalize(-light.direction);
    
    // 计算聚光灯角度
    float theta = dot(lightDir, spotDir);
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // 距离衰减计算
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                             light.quadratic * (distance * distance));
    
    // 阴影计算
    float shadow = 0.0;
    if (shadowEnabled && light.hasShadows) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace, lightShadowMap);
    }
    
    // 漫反射分量
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * albedo;
    
    // 镜面反射分量（Blinn-Phong）
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), SHININESS_FACTOR);
    vec3 specular = light.specular * spec * specularColor;
    
    // 环境光分量
    vec3 ambient = light.ambient * ambientColor * ao;

    // 应用衰减、聚光强度和阴影
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity * (1.0 - shadow);
    specular *= attenuation * intensity * (1.0 - shadow);
    
    return ambient + diffuse + specular;
}