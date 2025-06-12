#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DMS screenTextureMS;
uniform sampler2DMS bloomBlurMS;

void main()
{
    vec3 sceneColor = vec3(0.0);
    vec3 bloomColor = vec3(0.0);
    for(int s = 0; s < 4; s++)
    {
        sceneColor += texelFetch(screenTextureMS, ivec2(TexCoords * textureSize(screenTextureMS)), s).rgb; // 采样多重采样纹理
        bloomColor += texelFetch(bloomBlurMS, ivec2(TexCoords * textureSize(bloomBlurMS)), s).rgb; // 采样模糊纹理
    }
    vec3 col = sceneColor + bloomColor; // 合并场景颜色和模糊颜色
    col /= 4.0; // 平均化多重采样的颜色
    col = col / (col + vec3(1.0));
    float gamma = 2.2;
    col = pow(col, vec3(1.0 / gamma)); // 伽马校正
    FragColor = vec4(col, 1.0);
}