#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;
uniform sampler2D gMetallic;
uniform sampler2D gRoughness;
uniform sampler2D gAo;
uniform sampler2D gAmbient;

// 相机
uniform vec3 viewPos;

// 光源结构
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
    bool shadowEnabled;
    sampler2D shadowMap;
    mat4 lightSpaceMatrix;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    bool shadowEnabled;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
    bool shadowEnabled;
    sampler2D shadowMap;
    mat4 lightSpaceMatrix;
};

// 光源数组
#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 32
#define MAX_SPOT_LIGHTS 32

uniform DirectionalLight dirLights[MAX_DIR_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int numLights[3]; // [dirLights, pointLights, spotLights]

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform bool iblEnabled;

// 阴影
uniform bool shadowEnabled;

const float PI = 3.14159265359;

// PBR函数
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0, vec3 fragPos)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    float shadow = 0.0;
    if (light.shadowEnabled && shadowEnabled) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace, light.shadowMap);
    }
    
    float NDF = DistributionGGX(normal, halfwayDir, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 radiance = light.color * light.intensity;
    
    return (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0, vec3 fragPos)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    float NDF = DistributionGGX(normal, halfwayDir, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 radiance = light.color * light.intensity * attenuation;
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0, vec3 fragPos)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    float shadow = 0.0;
    if (light.shadowEnabled && shadowEnabled) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace, light.shadowMap);
    }
    
    float NDF = DistributionGGX(normal, halfwayDir, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 radiance = light.color * light.intensity * attenuation * intensity;
    
    return (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);
}

void main()
{
    // 从G-Buffer获取数据
    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 Normal = texture(gNormal, TexCoord).rgb;
    vec4 AlbedoSpec = texture(gAlbedo, TexCoord);
    vec3 Albedo = AlbedoSpec.rgb;
    
    // 检查是否为PBR材质 (gAlbedo.a == 1.0表示PBR材质)
    if (AlbedoSpec.a < 0.5) {
        // 非PBR材质，直接输出环境光
        FragColor = vec4(Albedo * 0.1, 1.0);
        return;
    }
    
    float Metallic = texture(gMetallic, TexCoord).r;
    float Roughness = texture(gRoughness, TexCoord).r;
    float AO = texture(gAo, TexCoord).r;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 计算F0
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, Albedo, Metallic);
    
    // 直接照明
    vec3 Lo = vec3(0.0);
    
    // 方向光
    for(int i = 0; i < numLights[0]; ++i) {
        Lo += CalcDirLight(dirLights[i], Normal, viewDir, Albedo, Metallic, Roughness, F0, FragPos);
    }
    
    // 点光源
    for(int i = 0; i < numLights[1]; ++i) {
        Lo += CalcPointLight(pointLights[i], Normal, viewDir, Albedo, Metallic, Roughness, F0, FragPos);
    }
    
    // 聚光灯
    for(int i = 0; i < numLights[2]; ++i) {
        Lo += CalcSpotLight(spotLights[i], Normal, viewDir, Albedo, Metallic, Roughness, F0, FragPos);
    }
    
    // 环境光照 (IBL)
    vec3 ambient = vec3(0.03) * Albedo * AO;
    if (iblEnabled) {
        vec3 F = fresnelSchlickRoughness(max(dot(Normal, viewDir), 0.0), F0, Roughness);
        
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - Metallic;
        
        vec3 irradiance = texture(irradianceMap, Normal).rgb;
        vec3 diffuse = irradiance * Albedo;
        
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, reflect(-viewDir, Normal), Roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(Normal, viewDir), 0.0), Roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
        
        ambient = (kD * diffuse + specular) * AO;
    }
    
    vec3 color = ambient + Lo;
    
    // HDR色调映射和gamma校正
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}
