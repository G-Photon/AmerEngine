#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom;
uniform bool hdrEnabled;
uniform bool bloomEnabled;
uniform bool gammaEnabled;
uniform float exposure = 1.0;
uniform float bloomIntensity = 1.0; // 新增

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;
    
    // Bloom处理 (带强度控制)
    vec3 bloomColor = bloomEnabled ? texture(bloom, TexCoords).rgb * bloomIntensity : vec3(0.0);
    if (!hdrEnabled) bloomColor = min(bloomColor, vec3(1.0)); // LDR保护
    color += bloomColor;

    // HDR色调映射
    if (hdrEnabled)
        color = vec3(1.0) - exp(-color * exposure);
    
    // Gamma校正 (最后执行)
    if (gammaEnabled)
        color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, 1.0);
}