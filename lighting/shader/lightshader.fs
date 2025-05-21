#version 330 core
out vec4 FragColor;
uniform vec3 lightColor; // uniform变量，表示光源的颜色
void main()
{
    FragColor = vec4(lightColor,1.0f);
}