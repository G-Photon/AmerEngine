#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
// layout (location = 4) in vec3 aBitangent;


out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 texCoords;
    mat4 model;
    mat4 view;
    mat4 projection;
    mat3 TBN;
} vs_out;

uniform mat4 model;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main()
{
    vs_out.FragPos = vec3(model * (vec4(aPos, 1.0)+ vec4(gl_InstanceID, 0.0, 0.0, 0.0)));
    vs_out.Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    vec3 T = normalize(mat3(model) * aTangent);
    T = T - dot(T, vs_out.Normal) * vs_out.Normal; // orthogonalize tangent
    T = normalize(T);
    vec3 B = normalize(mat3(model) * cross(aNormal, aTangent));
    vs_out.TBN = mat3(T, B, vs_out.Normal);
    vs_out.texCoords = aTexCoords;
    vs_out.model = model;
    vs_out.view = view;
    vs_out.projection = projection;

    gl_Position = vec4(aPos, 1.0) + vec4(gl_InstanceID, 0.0, 0.0,0.0);
}