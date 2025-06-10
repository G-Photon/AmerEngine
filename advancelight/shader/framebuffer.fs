#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DMS screenTextureMS;

const float offset = 1.0 / 300.0;  

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // 左上
        vec2( 0.0f,    offset), // 正上
        vec2( offset,  offset), // 右上
        vec2(-offset,  0.0f),   // 左
        vec2( 0.0f,    0.0f),   // 中
        vec2( offset,  0.0f),   // 右
        vec2(-offset, -offset), // 左下
        vec2( 0.0f,   -offset), // 正下
        vec2( offset, -offset)  // 右下
    );

    float kernel[9] = float[](
    0.0 / 16, 0.0 / 16, 0.0 / 16,
    0.0 / 16, 16.0 / 16, 0.0 / 16,
    0.0 / 16, 0.0 / 16, 0.0 / 16  
    );
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        // 对每个采样点取所有MSAA样本的平均值
        vec3 color = vec3(0.0);
        for(int s = 0; s < 4; s++)
        {
            color += texelFetch(screenTextureMS, ivec2((TexCoords.st + offsets[i]) * textureSize(screenTextureMS)), s).rgb;
        }
        sampleTex[i] = color / float(4);
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    col = col / (col + vec3(1.0));
    float gamma = 2.2;
    col = pow(col, vec3(1.0 / gamma)); // 伽马校正
    FragColor = vec4(col, 1.0);
}