#version 430 core
out vec4 FragColor;

uniform vec3 u_LightColor;

void main()
{
    FragColor = vec4(u_LightColor, 1.0);
    
    // 可选：添加简单衰减效果
    float distance = length(gl_PointCoord - vec2(0.5));
    float falloff = 1.0 - smoothstep(0.3, 0.5, distance);
    FragColor.a = falloff;
}