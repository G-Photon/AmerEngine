#version 430 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 resolution;

#define EDGE_THRESHOLD 0.125
#define EDGE_THRESHOLD_MIN 0.0625

void main() {
    vec2 invRes = 1.0 / resolution;
    
    // 当前像素颜色
    vec3 rgbNW = texture(screenTexture, TexCoords + vec2(-1.0, -1.0) * invRes).xyz;
    vec3 rgbNE = texture(screenTexture, TexCoords + vec2(1.0, -1.0) * invRes).xyz;
    vec3 rgbSW = texture(screenTexture, TexCoords + vec2(-1.0, 1.0) * invRes).xyz;
    vec3 rgbSE = texture(screenTexture, TexCoords + vec2(1.0, 1.0) * invRes).xyz;
    vec3 rgbM  = texture(screenTexture, TexCoords).xyz;
    
    // 计算亮度
    float lumaNW = dot(rgbNW, vec3(0.299, 0.587, 0.114));
    float lumaNE = dot(rgbNE, vec3(0.299, 0.587, 0.114));
    float lumaSW = dot(rgbSW, vec3(0.299, 0.587, 0.114));
    float lumaSE = dot(rgbSE, vec3(0.299, 0.587, 0.114));
    float lumaM  = dot(rgbM,  vec3(0.299, 0.587, 0.114));
    
    // 确定边缘方向
    float edgeVert = 
        abs((0.25 * lumaNW) + (-0.5 * lumaM) + (0.25 * lumaNE)) +
        abs((0.50 * lumaSW) + (-1.0 * lumaM) + (0.50 * lumaSE));
    
    float edgeHorz = 
        abs((0.25 * lumaNW) + (-0.5 * lumaM) + (0.25 * lumaSW)) +
        abs((0.50 * lumaNE) + (-1.0 * lumaM) + (0.50 * lumaSE));
    
    bool isHorizontal = edgeHorz >= edgeVert;
    
    // 选择采样方向
    float luma1 = isHorizontal ? lumaSW : lumaSE;
    float luma2 = isHorizontal ? lumaNW : lumaNE;
    
    // 计算梯度
    float gradient1 = luma1 - lumaM;
    float gradient2 = luma2 - lumaM;
    
    bool is1Steepest = abs(gradient1) >= abs(gradient2);
    float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
    
    // 计算步长
    float stepLength = isHorizontal ? invRes.y : invRes.x;
    
    // 计算UV偏移
    float lumaLocalAvg = 0.0;
    if (is1Steepest) {
        stepLength = -stepLength;
        lumaLocalAvg = 0.5 * (luma1 + lumaM);
    } else {
        lumaLocalAvg = 0.5 * (luma2 + lumaM);
    }
    
    // 计算新UV坐标
    vec2 currentUv = TexCoords;
    if (isHorizontal) {
        currentUv.y += stepLength * 0.5;
    } else {
        currentUv.x += stepLength * 0.5;
    }
    
    // 采样新位置
    vec3 result = texture(screenTexture, currentUv).xyz;
    
    // 计算最终亮度
    float lumaAvg = dot(result, vec3(0.299, 0.587, 0.114));
    
    // 混合条件
    if ((lumaAvg < lumaLocalAvg) || (lumaAvg > max(lumaM, lumaLocalAvg))) {
        FragColor = vec4(rgbM, 1.0);
    } else {
        FragColor = vec4(result, 1.0);
    }
}