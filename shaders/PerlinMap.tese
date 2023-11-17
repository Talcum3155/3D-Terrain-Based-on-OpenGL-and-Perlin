#version 460 core

// sepcify patch type, spacing tyep, winding order for the generated primitives
layout (quads, fractional_odd_spacing, ccw) in;

uniform sampler2D height_map;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec2 texture_coord[];

out float height;

void main() {
    // the coordinates of the center of gravity
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Retrieve the four texture coordinates of corners of the panel
    vec2 t00 = texture_coord[0];
    vec2 t01 = texture_coord[1];
    vec2 t10 = texture_coord[2];
    vec2 t11 = texture_coord[3];

    // bilinearly interpolate texture coodinate across patch
    // interpolate the coordinates of lower edge along the u-axis
    vec2 t0 = (t01 - t00) * u + t00;
    // interpolate the coordinates of upper edge along the u-axis
    vec2 t1 = (t11 - t10) * u + t10;
    // interpolate the real coordinates of texture along the v-axis
    vec2 tex_coord = (t1 - t0) * v + t0;

    height = texture(height_map, tex_coord).y * 64.0f - 16.0f;

    // Retrieve the four model coordinates of corners of the panel
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    // vector of u
    vec4 u_vector = p01 - p00;
    // vector of v
    vec4 v_vector = p10 - p00;
    // compute normal
    vec4 normal = normalize(vec4(cross(v_vector.xyz, u_vector.xyz), 0));

    // bilinearly interpolate model coodinates across patch
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0 + normal * height;

    // perform the MVP (Model-View-Projection) transformation.
    gl_Position = projection * view * model * p;
}