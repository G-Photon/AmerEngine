#version 460 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gMRao;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    
    sampler2D albedoMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D aoMap;
    sampler2D normalMap;
    
    bool useAlbedoMap;
    bool useMetallicMap;
    bool useRoughnessMap;
    bool useAoMap;
    bool useNormalMap;
};

uniform Material material;

void main() {
    // 存储片段位置向量和深度
    gPosition = vec4(FragPos, gl_FragCoord.z);
    
    // 存储法线向量
    vec3 N = normalize(Normal);
    if (material.useNormalMap) {
        // 从法线贴图获取法线
        vec3 tangentNormal = texture(material.normalMap, TexCoords).xyz * 2.0 - 1.0;
        
        // 创建TBN矩阵
        vec3 T = normalize(Tangent);
        vec3 B = normalize(Bitangent);
        mat3 TBN = mat3(T, B, N);
        N = normalize(TBN * tangentNormal);
    }
    gNormal = vec4(N, 1.0);
    
    // 存储反照率
    vec3 albedo = material.useAlbedoMap ? 
        texture(material.albedoMap, TexCoords).rgb : material.albedo;
    gAlbedo = vec4(albedo, 1.0);
    
    // 存储金属度、粗糙度和AO
    float metallic = material.useMetallicMap ? 
        texture(material.metallicMap, TexCoords).r : material.metallic;
    float roughness = material.useRoughnessMap ? 
        texture(material.roughnessMap, TexCoords).r : material.roughness;
    float ao = material.useAoMap ? 
        texture(material.aoMap, TexCoords).r : material.ao;
    
    gMRao = vec4(metallic, roughness, ao, 1.0);
}