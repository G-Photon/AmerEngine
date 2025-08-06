#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 Normal;
    mat3 TBN;
} fs_in;

out vec4 FragColor;

// 材质结构
struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    
    bool useAlbedoMap;
    bool useMetallicMap;
    bool useRoughnessMap;
    bool useNormalMap;
    bool useAOMap;
    
    sampler2D albedoMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D normalMap;
    sampler2D aoMap;
};

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

// Uniforms
uniform Material material;
uniform vec3 viewPos;

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

// PBR函数声明
vec3 getNormalFromMap();
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap);
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0);

// 获取法线贴图的法线
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(material.normalMap, fs_in.TexCoord).xyz * 2.0 - 1.0;
    return normalize(fs_in.TBN * tangentNormal);
}

// Normal Distribution Function
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

// Geometry function
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

// Fresnel function
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 阴影计算
float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap)
{
    // 透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    // 获取最近点的深度值
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 获取当前片段的深度值
    float currentDepth = projCoords.z;

    // 偏移量以减少阴影失真
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

// 计算方向光
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    // 计算阴影
    float shadow = 0.0;
    if (light.shadowEnabled && shadowEnabled) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fs_in.FragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace, light.shadowMap);
    }
    
    // Cook-Torrance BRDF
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

// 计算点光源
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    float distance = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    // Cook-Torrance BRDF
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

// 计算聚光灯
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    float distance = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    // 计算阴影
    float shadow = 0.0;
    if (light.shadowEnabled && shadowEnabled) {
        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fs_in.FragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace, light.shadowMap);
    }
    
    // Cook-Torrance BRDF
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
    // 获取材质属性
    vec3 albedo = material.useAlbedoMap ? texture(material.albedoMap, fs_in.TexCoord).rgb : material.albedo;
    float metallic = material.useMetallicMap ? texture(material.metallicMap, fs_in.TexCoord).r : material.metallic;
    float roughness = material.useRoughnessMap ? texture(material.roughnessMap, fs_in.TexCoord).r : material.roughness;
    float ao = material.useAOMap ? texture(material.aoMap, fs_in.TexCoord).r : material.ao;
    
    // 获取法线
    vec3 normal = material.useNormalMap ? getNormalFromMap() : normalize(fs_in.Normal);
    
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    
    // 计算F0
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // 直接照明
    vec3 Lo = vec3(0.0);
    
    // 方向光
    for(int i = 0; i < numLights[0]; ++i) {
        Lo += CalcDirLight(dirLights[i], normal, viewDir, albedo, metallic, roughness, F0);
    }
    
    // 点光源
    for(int i = 0; i < numLights[1]; ++i) {
        Lo += CalcPointLight(pointLights[i], normal, viewDir, albedo, metallic, roughness, F0);
    }
    
    // 聚光灯
    for(int i = 0; i < numLights[2]; ++i) {
        Lo += CalcSpotLight(spotLights[i], normal, viewDir, albedo, metallic, roughness, F0);
    }
    
    // 环境光照 (IBL)
    vec3 ambient = vec3(0.03) * albedo * ao;
    if (iblEnabled) {
        vec3 F = fresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), F0, roughness);
        
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;
        
        vec3 irradiance = texture(irradianceMap, normal).rgb;
        vec3 diffuse = irradiance * albedo;
        
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, reflect(-viewDir, normal), roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
        
        ambient = (kD * diffuse + specular) * ao;
    }
    
    vec3 color = ambient + Lo;
    
    FragColor = vec4(color, 1.0);
}
