#version 330 core
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec4 texcoord;
out vec4 texCoords;
uniform mat4 mvp;
void main()
{
    gl_Position = mvp * vec4(vertex, 0.0, 1.0);
    texCoords = texcoord;
}