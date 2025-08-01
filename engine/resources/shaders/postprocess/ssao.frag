#version 430 core
out float FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform int kernelSize;
uniform float radius;
uniform float bias;
uniform float power;
uniform mat4 view;


uniform mat4 projection;
uniform vec2 noiseScale;

void main() {
    // 获取视图空间位置和法线
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    fragPos = vec3(view * vec4(fragPos, 1.0)); // 将位置转换到视图空间
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    normal = normalize(vec3(view * vec4(normal, 0.0))); // 将法线转换到视图空间
    
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
        
        // 获取实际场景深度（线性）
        float sceneDepth = texture(gPosition, offset.xy).w;
        float sampleDepth = -samplePos.z; // 视图空间深度
        
        // 深度比较 + 范围检查
        float rangeCheck = abs(fragPos.z - sceneDepth) < radius ? 1.0 : 0.0;
        occlusion += (sceneDepth < sampleDepth + bias) ? rangeCheck : 0.0;
    }
    
    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = pow(occlusion, power);
}