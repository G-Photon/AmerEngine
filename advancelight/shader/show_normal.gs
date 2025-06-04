#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 texCoords;
    mat4 model;
    mat4 view;
    mat4 projection;
} gs_in[];


vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(b, a));
}

void nochange(int index)
{
    mat4 mvp = gs_in[index].projection * gs_in[index].view * gs_in[index].model;
    gl_Position = mvp * gl_in[index].gl_Position;
    EmitVertex();
}

void show_normal(vec3 normal, int index)
{
    mat4 mvp = gs_in[index].projection * gs_in[index].view * gs_in[index].model;
    float magnitude = 0.1;
    gl_Position = mvp * (gl_in[index].gl_Position + vec4(normal * magnitude, 0.0));
    EmitVertex();
    EndPrimitive();
}
void main() {
    vec3 normal = GetNormal();
    nochange(0);
    show_normal(normal, 0);
    nochange(1);
    show_normal(normal, 1);
    nochange(2);
    show_normal(normal, 2);
}