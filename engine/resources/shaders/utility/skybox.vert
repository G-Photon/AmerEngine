#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    WorldPos = aPos;
    // 去掉平移分量
    mat4 rotView = mat4(mat3(view));
    vec4 clipPos = projection * rotView * vec4(aPos, 1.0);
    // 让 z 永远等于 w，保证深度始终为 1（天空盒应最远）
    gl_Position = clipPos.xyww;
}