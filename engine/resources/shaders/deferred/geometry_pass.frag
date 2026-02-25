#version 460 core
layout (location = 0) out vec4 gAlbedoSpec;      // RGB: Albedo, A: MaterialID (0.0 for BP)
layout (location = 1) out vec4 gNormalRoughness; // RGB: Normal, A: Unused
layout (location = 2) out vec4 gMRA;             // RGB: Specular Color, A: Unused

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float metallic; // 金属度
    float roughness; // 粗糙度
    float shininess;
    
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D normalMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D aoMap;

    bool useDiffuseMap;
    bool useSpecularMap;
    bool useNormalMap;
    bool useMetallicMap;
    bool useRoughnessMap;
    bool useAoMap;
};

uniform Material material;
uniform float NEAR= 0.1;
uniform float FAR= 100.0;
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // 回到NDC
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));    
}

void main() {
    // 法线计算 (世界空间)
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
    gNormalRoughness = vec4(N, 0.0); // Roughness unused (0.0)

    // RT0: 反照率 + ID (ID=0.0 for Blinn-Phong)
    vec3 albedo = material.useDiffuseMap ? texture(material.diffuseMap, TexCoords).rgb : material.diffuse;
    gAlbedoSpec = vec4(albedo, 0.0);
    
    // RT2: 高光颜色 (RGB) + Unused (A)
    vec3 specular = material.useSpecularMap ? texture(material.specularMap, TexCoords).rgb : material.specular;
    gMRA = vec4(specular, 1.0);
}
