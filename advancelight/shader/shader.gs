#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 texCoords;
    mat4 model;
    mat4 view;
    mat4 projection;
    mat3 TBN;
} gs_in[];

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out mat3 TBN;
uniform float time;

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(b, a));
}

void explode(vec3 normal, int index)
{
    mat4 mvp = gs_in[index].projection * gs_in[index].view * gs_in[index].model;
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    gl_Position = mvp * (gl_in[index].gl_Position + vec4(direction, 0.0));
    Normal = gs_in[index].Normal;
    FragPos = gs_in[index].FragPos;
    TexCoords = gs_in[index].texCoords;
    EmitVertex();
}

void nochange(int index)
{
    mat4 mvp = gs_in[index].projection * gs_in[index].view * gs_in[index].model;
    gl_Position = mvp * gl_in[index].gl_Position;
    TBN = gs_in[index].TBN;
    Normal = gs_in[index].Normal;
    FragPos = gs_in[index].FragPos;
    TexCoords = gs_in[index].texCoords;
    EmitVertex();
}

void show_normal(vec4 position, vec3 normal, int index)
{
    mat4 mvp = gs_in[index].projection * gs_in[index].view * gs_in[index].model;
    float magnitude = 0.1;
    gl_Position = mvp * (position + vec4(normal * magnitude, 0.0));
}
void main() {
    vec3 normal = GetNormal();
    // explode(normal, 0);
    // explode(normal, 1);
    // explode(normal, 2);
    nochange(0);
    nochange(1);
    nochange(2);
    EndPrimitive();
}