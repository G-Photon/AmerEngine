#version 330 core

layout (location = 0) out vec4 gAlbedoSpec;      // RGB: Albedo, A: MaterialID
layout (location = 1) out vec4 gNormalRoughness; // RGB: Normal, A: Roughness
layout (location = 2) out vec4 gMRA;             // R: Metallic, G: AO, B: Unused

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 Normal;
    mat3 TBN;
} fs_in;

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

uniform Material material;

// 获取法线贴图的法线
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(material.normalMap, fs_in.TexCoord).xyz * 2.0 - 1.0;
    return normalize(fs_in.TBN * tangentNormal);
}

void main()
{
    // 法线 (世界空间) + 粗糙度 -> RT1
    vec3 N = material.useNormalMap ? getNormalFromMap() : normalize(fs_in.Normal);
    float roughness = material.useRoughnessMap ? texture(material.roughnessMap, fs_in.TexCoord).r : material.roughness;
    gNormalRoughness = vec4(N, roughness);

    // 反照率 + ID -> RT0 (ID=1.0 for PBR)
    vec3 albedo = material.useAlbedoMap ? texture(material.albedoMap, fs_in.TexCoord).rgb : material.albedo;
    gAlbedoSpec = vec4(albedo, 1.0);
    
    // 金属度 + AO -> RT2 (R=Metallic, G=AO)
    float metallic = material.useMetallicMap ? texture(material.metallicMap, fs_in.TexCoord).r : material.metallic;
    float ao = material.useAOMap ? texture(material.aoMap, fs_in.TexCoord).r : material.ao;
    gMRA = vec4(metallic, ao, 0.0, 1.0);
}
