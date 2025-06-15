#version 330 core
in vec3 Veccolor;
out vec4 FragColor;
void main()
{
    FragColor = vec4(Veccolor, 1.0);
}