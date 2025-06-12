#version 410 core
#extension GL_ARB_sample_shading : enable
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2DMS imageMS;

uniform bool horizontal;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
    vec2 tex_offset = vec2(1.0); // gets size of single texel
    int sampleID = gl_SampleID;
    vec3 result = texelFetch(imageMS, ivec2(TexCoords* textureSize(imageMS)), sampleID).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texelFetch(imageMS, ivec2(TexCoords * textureSize(imageMS) + vec2(tex_offset.x * i, 0.0)), sampleID).rgb * weight[i];
            result += texelFetch(imageMS, ivec2(TexCoords * textureSize(imageMS) - vec2(tex_offset.x * i, 0.0)), sampleID).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texelFetch(imageMS, ivec2(TexCoords * textureSize(imageMS) + vec2(0.0, tex_offset.y * i)), sampleID).rgb * weight[i];
            result += texelFetch(imageMS, ivec2(TexCoords * textureSize(imageMS) - vec2(0.0, tex_offset.y * i)), sampleID).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}