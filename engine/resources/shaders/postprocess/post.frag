#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;   // HDRBuffer
uniform sampler2D bloom;   // BloomBlur
uniform sampler2D ssao;    // SSAO
uniform bool hdrEnabled;
uniform bool bloomEnabled;
uniform bool ssaoEnabled;
uniform bool gammaEnabled;
uniform float exposure = 1.0;

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;

    if (ssaoEnabled)
        color *= texture(ssao, TexCoords).r;

    if (bloomEnabled)
        color += texture(bloom, TexCoords).rgb;

    if (hdrEnabled)
        color = vec3(1.0) - exp(-color * exposure); // Reinhard

    if (gammaEnabled)
        color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}