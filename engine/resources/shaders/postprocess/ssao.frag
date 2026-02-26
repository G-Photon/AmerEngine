#version 430 core
out float FragColor;
in vec2 TexCoords;

uniform sampler2D gDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform int kernelSize;
uniform float radius;
uniform float bias;
uniform float power;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 invProjection;
uniform vec2 noiseScale;

// Octahedron-normal vectors decoding
vec3 octDecode(vec2 e) {
    vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
    if (v.z < 0.0) v.xy = (1.0 - abs(v.yx)) * sign(v.xy);
    return normalize(v);
}

vec3 ReconstructViewPos(vec2 texCoords, float depth) {
    vec4 clipSpacePosition = vec4(texCoords * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewSpacePosition = invProjection * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    return viewSpacePosition.xyz;
}

void main() {
    // 深度值
    float depth = texture(gDepth, TexCoords).r;
    if (depth >= 1.0) discard; // Skip background

    // 重建视图空间位置
    vec3 fragPos = ReconstructViewPos(TexCoords, depth);
    
    // 获取视图空间法线
    vec3 worldNormal = octDecode(texture(gNormal, TexCoords).rg);
    vec3 normal = normalize(vec3(view * vec4(worldNormal, 0.0))); 
    
    // 重建TBN矩阵

    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    // 计算环境光遮蔽
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; i++) {
        // 获取样本位置（视图空间）
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;
        
        // 投影到屏幕空间
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // 视图空间->裁剪空间
        offset.xyz /= offset.w; // 透视除法
        offset.xyz = offset.xyz * 0.5 + 0.5; // 变换到0.0-1.0
        
        // 获取样本深度
        float depthRaw = texture(gDepth, offset.xy).r;
        vec3 samplePosRec = ReconstructViewPos(offset.xy, depthRaw);
        float sampleDepth = samplePosRec.z;
        
        // 范围检查
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        
        // 累计遮蔽
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = occlusion;
}