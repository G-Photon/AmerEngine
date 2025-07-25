#version 430 core
uniform sampler2DMS uSceneMS;
uniform int         uSamples;
in vec2 TexCoords;
out vec4 FragColor;

vec4 resolveMS(sampler2DMS tex, ivec2 coord) {
    vec4 sum = vec4(0);
    for (int i = 0; i < uSamples; ++i)
        sum += texelFetch(tex, coord, i);
    return sum / float(uSamples);
}

void main() {
    ivec2 coord = ivec2(gl_FragCoord.xy);
    vec3 hdr = resolveMS(uSceneMS, coord).rgb;

    FragColor = vec4(hdr, 1.0);
}