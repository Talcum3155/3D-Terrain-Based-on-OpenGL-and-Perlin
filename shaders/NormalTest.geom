#version 460 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

const float MAGNITUDE = 2.0f;

uniform mat4 projection;

in VS_OUT {
    vec3 normal;
} gs_in[];

void GenerateLine(int index, vec3 normal)
{
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection *
    (gl_in[index].gl_Position + vec4(normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main() {
    GenerateLine(0, gs_in[0].normal); // first vertex normal
    GenerateLine(1, gs_in[1].normal); // second vertex normal
    GenerateLine(2, gs_in[2].normal); // third vertex normal
}