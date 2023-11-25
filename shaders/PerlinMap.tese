#version 460 core

// sepcify patch type, spacing tyep, winding order for the generated primitives
layout (quads, fractional_odd_spacing, ccw) in;

out terrain_data {
    float height; // real value of height
    float height_01; // height value range (0,1)
    vec2 tex_coord;
    vec3 frag_pos;
} data;

uniform sampler2D height_map;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float terrain_height;

in vec2 texture_coord_h[];
in vec2 texture_coord[];

vec2 interpolate_tex_coord(float u, float v, vec2 t00, vec2 t01, vec2 t10, vec2 t11) {

    // bilinearly interpolate texture coodinate across patch
    // interpolate the coordinates of lower edge along the u-axis
    vec2 t0 = (t01 - t00) * u + t00;
    // interpolate the coordinates of upper edge along the u-axis
    vec2 t1 = (t11 - t10) * u + t10;

    // interpolate the real coordinates of texture along the v-axis
    return (t1 - t0) * v + t0;
}

void main() {
    // the coordinates of the center of gravity
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Retrieve the four texture coordinates of corners of the panel
    vec2 tex_coord_h = interpolate_tex_coord(u, v, texture_coord_h[0], texture_coord_h[1], texture_coord_h[2], texture_coord_h[3]);

    data.height_01 = texture(height_map, tex_coord_h).x;
    data.height = data.height_01 * terrain_height - (terrain_height / 3.0f);

    data.tex_coord = interpolate_tex_coord(u, v, texture_coord[0], texture_coord[1], texture_coord[2], texture_coord[3]);

    // Retrieve the four model coordinates of corners of the panel
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    // bilinearly interpolate model coodinates across patch
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    //    vec4 p = (p1 - p0) * v + p0;
    vec4 p = (p1 - p0) * v + p0 + vec4(0, data.height, 0, 0);
    data.frag_pos = p.xyz;

    // perform the MVP (Model-View-Projection) transformation.
    gl_Position = projection * view * model * p;
}