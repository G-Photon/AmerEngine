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

void main()
{
    // 获取材质属性
    vec3 albedo = material.useAlbedoMap ? texture(material.albedoMap, fs_in.TexCoord).rgb : material.albedo;
    float metallic = material.useMetallicMap ? texture(material.metallicMap, fs_in.TexCoord).r : material.metallic;
    float roughness = material.useRoughnessMap ? texture(material.roughnessMap, fs_in.TexCoord).r : material.roughness;
    float ao = material.useAOMap ? texture(material.aoMap, fs_in.TexCoord).r : material.ao;
    
    // 简单的环境光着色
    vec3 color = albedo * 0.1 + albedo * metallic * 0.5 + vec3(roughness * 0.2);
    
    FragColor = vec4(color, 1.0);
}
