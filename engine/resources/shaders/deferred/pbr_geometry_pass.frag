#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gSpecular;
layout (location = 4) out float gMetallic;
layout (location = 5) out float gRoughness;
layout (location = 6) out float gAo;
layout (location = 7) out vec3 gAmbient;

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
    // 位置 (世界空间)
    gPosition = fs_in.FragPos;
    
    // 法线 (世界空间)
    gNormal = material.useNormalMap ? getNormalFromMap() : normalize(fs_in.Normal);
    
    // 反照率
    gAlbedo.rgb = material.useAlbedoMap ? texture(material.albedoMap, fs_in.TexCoord).rgb : material.albedo;
    gAlbedo.a = 1.0; // 标记为PBR材质
    
    // 对于PBR，高光颜色设置为0，我们将使用金属度和粗糙度
    gSpecular = vec3(0.0);
    
    // PBR参数
    gMetallic = material.useMetallicMap ? texture(material.metallicMap, fs_in.TexCoord).r : material.metallic;
    gRoughness = material.useRoughnessMap ? texture(material.roughnessMap, fs_in.TexCoord).r : material.roughness;
    gAo = material.useAOMap ? texture(material.aoMap, fs_in.TexCoord).r : material.ao;
    
    // 环境光，暂时设为默认值
    gAmbient = vec3(0.1);
}
