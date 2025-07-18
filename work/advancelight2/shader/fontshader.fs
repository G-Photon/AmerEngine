#version 330 core
in vec4 texCoords;
out vec4 FragColor;
uniform sampler2D textTexture;
// uniform vec3 texColor;
uniform float deltaTime;
uniform float thickness;
uniform float softness;
uniform float outline_thickness;
uniform float outline_softness;
uniform vec2 shadow_offset;
void main()
{
    vec2 TexCoords = texCoords.xy;
    float alpha = texture(textTexture, TexCoords).r;
    float offset_alpha = texture(textTexture, (TexCoords + shadow_offset)).r;
    // ---- 
    vec3 texColor = vec3(0.666, .0, 1.0);
    // vec3 texColor = vec3(0.761, 0.165, 0.839);
    vec3 outline_color = vec3(0.0, 1.0, 0.8);
    vec3 shadow_color = vec3(.0);
    vec3 gradients_color = vec3(0.631, 0.863, 0.333);
    // ----
    float shadow_alpha = smoothstep(1.0 - thickness - softness, 1.0 - thickness, offset_alpha);
    float text_alpha = smoothstep(1.0 - thickness - softness, 1.0 - thickness, alpha);
    // ----
    float outline = smoothstep(outline_thickness - outline_softness, outline_thickness + outline_softness, alpha);

    float result = mod((texCoords.z + texCoords.w) * 0.5 + deltaTime, 2.0);
    result = (result > 1.0 ) ? (2.0 - result) : result;
    // vec3 text_color = mix(outline_color, mix(gradients_color, texColor, fract(result)), outline);
    // smooth a little
    result = fract(result);
    vec3 text_color = mix(outline_color, mix(gradients_color, texColor, result * result * (3.0 - 2.0 * result) ), outline);

    float overallAlpha = text_alpha + (1 - text_alpha) * shadow_alpha;
    FragColor = vec4(mix(shadow_color, text_color, text_alpha / overallAlpha), overallAlpha);
}
