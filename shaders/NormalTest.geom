#version 460 core

layout (triangles) in;
layout (line_strip, max_vertices = 18) out;

const float MAGNITUDE = 2.0f;

uniform mat4 projection;
uniform mat4 view;

in VS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    mat3 tangent_space;
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

//    GenerateLine(0, gs_in[0].tangent);
//    GenerateLine(1, gs_in[1].tangent);
//    GenerateLine(2, gs_in[2].tangent);
//
//    GenerateLine(0, gs_in[0].bitangent);
//    GenerateLine(1, gs_in[1].bitangent);
//    GenerateLine(2, gs_in[2].bitangent);
}