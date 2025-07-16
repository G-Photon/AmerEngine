    #version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMRao;

struct DirLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 4
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_POINT_LIGHTS];

uniform vec3 viewPos;
uniform bool useIBL;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

const float PI = 3.14159265359;

// PBR光照计算函数
vec3 CalculateDirLight(DirLight light, vec3 normal, vec3 viewDir, 
                      vec3 albedo, float metallic, float roughness, float ao, vec3 F0);
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,
                        vec3 albedo, float metallic, float roughness, float ao, vec3 F0);
vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir,
                       vec3 albedo, float metallic, float roughness, float ao, vec3 F0);

// IBL函数
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 CalculateIBL(vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, float ao);

void main() {
    // 从G缓冲中获取数据
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
    float Metallic = texture(gMRao, TexCoords).r;
    float Roughness = texture(gMRao, TexCoords).g;
    float Ao = texture(gMRao, TexCoords).b;
    
    // 输入光照数据
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-V, N);
    
    // 计算反射率
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, Albedo, Metallic);
    
    // 反射光方程
    vec3 Lo = vec3(0.0);
    
    // 方向光
    Lo += CalculateDirLight(dirLight, N, V, Albedo, Metallic, Roughness, Ao, F0);
    
    // 点光源
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        Lo += CalculatePointLight(pointLights[i], N, FragPos, V, Albedo, Metallic, Roughness, Ao, F0);
    }
    
    // 聚光灯
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        Lo += CalculateSpotLight(spotLights[i], N, FragPos, V, Albedo, Metallic, Roughness, Ao, F0);
    }
    
    // 环境光 (IBL)
    vec3 ambient = vec3(0.03) * Albedo * Ao;
    if (useIBL) {
        ambient = CalculateIBL(N, V, Albedo, Metallic, Roughness, Ao);
    }
    
    vec3 color = ambient + Lo;
    
    // HDR色调映射
    color = color / (color + vec3(1.0));
    // gamma校正
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}

// 实现各种光照计算函数...
vec3 CalculateDirLight(DirLight light, vec3 normal, vec3 viewDir, 
                      vec3 albedo, float metallic, float roughness, float ao, vec3 F0) {
    // 计算漫反射和镜面反射
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * albedo;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 1.0 / (roughness * roughness));
    vec3 specular = spec * light.color * F0;
    
    return (diffuse + specular) * ao;
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,
                        vec3 albedo, float metallic, float roughness, float ao, vec3 F0) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                              light.quadratic * (distance * distance));
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * albedo * attenuation;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 1.0 / (roughness * roughness));
    vec3 specular = spec * light.color * F0 * attenuation;
    
    return (diffuse + specular) * ao;
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir,
                       vec3 albedo, float metallic, float roughness, float ao, vec3 F0) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                              light.quadratic * (distance * distance));
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * albedo * attenuation * intensity;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 1.0 / (roughness * roughness));
    vec3 specular = spec * light.color * F0 * attenuation * intensity;
    
    return (diffuse + specular) * ao;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), vec3(0.0)) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalculateIBL(vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, float ao) {
    // 计算环境光照
    vec3 irradiance = texture(irradianceMap, normal).rgb;
    
    // 预过滤环境贴图
    vec3 prefilteredColor = textureLod(prefilterMap, reflect(-viewDir, normal), roughness * 128.0).rgb;
    
    // BRDF计算
    vec2 brdf = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
    
    // 环境光照
    return (irradiance * albedo + prefilteredColor * brdf.x) * ao;
}