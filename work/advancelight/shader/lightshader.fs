#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
uniform vec3 lightColor; // uniform变量，表示光源的颜色
void main()
{
    FragColor = vec4(200,200,200,1.0f);
    BrightColor = FragColor;
}