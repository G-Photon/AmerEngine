#version 430 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 resolution;

#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0

void main() {
    vec2 invRes = 1.0 / resolution;
    
    // 当前像素颜色
    vec3 rgbNW = texture(screenTexture, TexCoords + vec2(-1.0, -1.0) * invRes).rgb;
    vec3 rgbNE = texture(screenTexture, TexCoords + vec2(1.0, -1.0) * invRes).rgb;
    vec3 rgbSW = texture(screenTexture, TexCoords + vec2(-1.0, 1.0) * invRes).rgb;
    vec3 rgbSE = texture(screenTexture, TexCoords + vec2(1.0, 1.0) * invRes).rgb;
    vec3 rgbM  = texture(screenTexture, TexCoords).rgb;
    
    // 计算亮度
    float lumaNW = dot(rgbNW, vec3(0.299, 0.587, 0.114));
    float lumaNE = dot(rgbNE, vec3(0.299, 0.587, 0.114));
    float lumaSW = dot(rgbSW, vec3(0.299, 0.587, 0.114));
    float lumaSE = dot(rgbSE, vec3(0.299, 0.587, 0.114));
    float lumaM  = dot(rgbM,  vec3(0.299, 0.587, 0.114));
    
    // 确定亮度范围
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    // 如果对比度不足，直接返回原始颜色
    if (lumaMax - lumaMin <= max(FXAA_REDUCE_MIN, lumaMax * FXAA_REDUCE_MUL)) {
        FragColor = vec4(rgbM, 1.0);
        return;
    }
    
    // 计算梯度方向
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    // 标准化方向
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), 
              max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * invRes;
    
    // 沿边缘采样
    vec3 rgbA = 0.5 * (
        texture(screenTexture, TexCoords + dir * (1.0/3.0 - 0.5)).rgb +
        texture(screenTexture, TexCoords + dir * (2.0/3.0 - 0.5)).rgb);
    
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(screenTexture, TexCoords + dir * -0.5).rgb +
        texture(screenTexture, TexCoords + dir * 0.5).rgb);
    
    // 计算混合后颜色的亮度
    float lumaB = dot(rgbB, vec3(0.299, 0.587, 0.114));
    
    // 确定是否使用混合结果
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        FragColor = vec4(rgbA, 1.0);
    } else {
        FragColor = vec4(rgbB, 1.0);
    }
}