#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

void main()
{
    gl_Position = u_ViewProjection * u_Model * vec4(aPos, 1.0);
}