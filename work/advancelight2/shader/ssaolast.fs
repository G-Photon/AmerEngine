#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;
uniform float ambientStrength = 0.3;

void main() {
    vec3 diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float ao = texture(ssao, TexCoords).r;
    
    // 只计算环境光并应用SSAO
    vec3 ambient = ambientStrength * diffuse * ao;
    float gamma=2.2;
    ambient =pow(ambient,vec3(1/gamma));
    FragColor = vec4(ambient, 1.0);
}