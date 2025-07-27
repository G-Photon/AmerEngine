#version 430 core
layout (location = 0) in vec3 aPos;

uniform int lightType; // 0:方向光, 1:点光源, 2:聚光灯
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

void main()
{
    TexCoords = aPos.xy * 0.5 + 0.5; // 将坐标转换到[0, 1]范围
    if (lightType == 1) {
        gl_Position = vec4(aPos, 1.0);
    } else {
        // 点光源和聚光灯：应用模型、视图和投影变换
        vec4 worldPos = model * vec4(aPos, 1.0);
        gl_Position = projection * view * worldPos;
    }
}