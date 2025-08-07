#version 430 core
out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube skybox;
uniform bool gammaEnabled;

void main()
{
    vec3 envColor = texture(skybox, WorldPos).rgb;

    // 简单处理HDR数据 - 使用简单的曝光控制
    // float exposure = 1.0;
    // envColor = vec3(1.0) - exp(-envColor * exposure);
    
    // Gamma校正
    // if (gammaEnabled) {
    //     envColor = pow(envColor, vec3(2.2));
    // }
    
    FragColor = vec4(envColor, 1.0);
}