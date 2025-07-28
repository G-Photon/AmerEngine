#version 430 core
layout (location = 0) in vec3 aPos;

uniform int lightType; // 0:点光源, 1:方向光, 2:聚光灯
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    if (lightType == 1) {
        gl_Position = vec4(aPos, 1.0);
    } else {
        // 点光源和聚光灯：应用模型、视图和投影变换
        vec4 worldPos = model * vec4(aPos, 1.0);
        gl_Position = projection * view * worldPos;
    }
}