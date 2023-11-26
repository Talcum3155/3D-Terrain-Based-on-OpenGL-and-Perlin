#version 460 core

// sepcify patch type, spacing tyep, winding order for the generated primitives
layout (quads, fractional_odd_spacing, ccw) in;

out terrain_data {
    float height; // real value of height
    float height_01; // height value range (0,1)
    vec2 tex_coord;
    vec3 frag_pos;
    vec3 normal;
} data;

uniform sampler2D height_map;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float terrain_height;
uniform float y_value;
uniform float HEIGHT_SCALE;

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

void calculate_normal_3(vec2 tex_coord) {

    float uTexelSize = 1.0 / 512.0;
    float vTexelSize = 1.0 / 512.0;

    // Sample heights around the current texture coordinate
    float left = texture(height_map, tex_coord + vec2(-uTexelSize, 0.0)).x * HEIGHT_SCALE * 2.0 - 1.0;
    float right = texture(height_map, tex_coord + vec2(uTexelSize, 0.0)).x * HEIGHT_SCALE * 2.0 - 1.0;
    float up = texture(height_map, tex_coord + vec2(0.0, vTexelSize)).x * HEIGHT_SCALE * 2.0 - 1.0;
    float down = texture(height_map, tex_coord + vec2(0.0, -vTexelSize)).x * HEIGHT_SCALE * 2.0 - 1.0;

    // Calculate normals by taking cross products and averaging

    data.normal = normalize(vec3(left - right, y_value, down - up));
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    data.normal = normalize(normalMatrix * data.normal);
}

void main() {
    // the coordinates of the center of gravity
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Retrieve uv of the four texture coordinates of corners of the panel
    vec2 tex_coord_h = interpolate_tex_coord(u, v, texture_coord_h[0], texture_coord_h[1], texture_coord_h[2], texture_coord_h[3]);

    data.height_01 = texture(height_map, tex_coord_h).x;
    data.height = data.height_01 * terrain_height - (terrain_height / 3.0f);

    // Retrieve the uv of texture
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

    // -----------------------------------------------------
    // must do model transform !!!
    data.frag_pos = vec3(model * p);
    // -----------------------------------------------------

    calculate_normal_3(tex_coord_h);
    //    calculate_normal_2(tex_coord_h);

    // perform the MVP (Model-View-Projection) transformation.
    gl_Position = projection * view * model * p;
}