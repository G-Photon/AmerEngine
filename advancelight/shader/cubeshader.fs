#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    float gamma=2.2;
    FragColor = pow(texture(skybox, TexCoords), vec4(gamma));
}