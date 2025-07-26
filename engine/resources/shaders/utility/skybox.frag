#version 430 core
out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform bool gammaEnabled;

void main()
{
    vec3 envColor = texture(environmentMap, WorldPos).rgb;

    // // 简单的 gamma 校正（可根据 Renderer 的 gamma 开关再调整）
    if (gammaEnabled) {
        envColor = pow(envColor, vec3(2.2));
    }
    
    FragColor = vec4(envColor, 1.0);
}