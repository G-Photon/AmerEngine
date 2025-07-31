#version 460 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gMetallic; // 金属度
layout (location = 5) out vec4 gRoughness; // 粗糙度
layout (location = 6) out vec4 gAo; // 环境光遮蔽
layout (location = 7) out vec4 gAmbient; // 环境光

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
    gAlbedo = vec4(material.useDiffuseMap ? 
        texture(material.diffuseMap, TexCoords).rgb : material.diffuse, 1.0);
    
    // 存储金属度、粗糙度和AO
    gSpecular = vec4(material.useSpecularMap ? 
        texture(material.specularMap, TexCoords).rgb : material.specular, 1.0);
    gMetallic = vec4(material.useMetallicMap ? 
        texture(material.metallicMap, TexCoords).r : material.metallic, 0.0, 0.0, 1.0);
    gRoughness = vec4(material.useRoughnessMap ?
        texture(material.roughnessMap, TexCoords).r : material.roughness, 0.0, 0.0, 1.0);
    gAo = vec4(material.useAoMap ?
        texture(material.aoMap, TexCoords).r : 1.0, 0.0, 0.0, 1.0);
    gAmbient = vec4(material.useDiffuseMap ?
        texture(material.diffuseMap, TexCoords).rgb : material.diffuse, 1.0);
}