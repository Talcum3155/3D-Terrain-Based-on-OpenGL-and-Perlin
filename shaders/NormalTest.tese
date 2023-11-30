#version 460 core

// sepcify patch type, spacing tyep, winding order for the generated primitives
layout (quads, fractional_odd_spacing, ccw) in;

uniform sampler2D height_map;
uniform mat4 model;
uniform mat4 view;

uniform float y_value;
uniform float HEIGHT_SCALE;

in vec2 texture_coord[];

out VS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    mat3 tangent_space;
} vs_out;

uniform float terrain_height;

void calculate_normal_3(vec2 tex_coord) {

    float uTexelSize = 1.0 / 256.0;
    float vTexelSize = 1.0 / 256.0;

    // Sample heights around the current texture coordinate
    float left = texture(height_map, tex_coord + vec2(-uTexelSize, 0.0)).x * HEIGHT_SCALE * 2.0 - 1.0;
    float right = texture(height_map, tex_coord + vec2(uTexelSize, 0.0)).x * HEIGHT_SCALE * 2.0 - 1.0;
    float up = texture(height_map, tex_coord + vec2(0.0, vTexelSize)).x * HEIGHT_SCALE * 2.0 - 1.0;
    float down = texture(height_map, tex_coord + vec2(0.0, -vTexelSize)).x * HEIGHT_SCALE * 2.0 - 1.0;

    // Diagonal samples
    float up_left = texture(height_map, tex_coord + vec2(-uTexelSize, vTexelSize)).x;
    float up_right = texture(height_map, tex_coord + vec2(uTexelSize, vTexelSize)).x;
    float down_left = texture(height_map, tex_coord + vec2(-uTexelSize, -vTexelSize)).x;
    float down_right = texture(height_map, tex_coord + vec2(uTexelSize, -vTexelSize)).x;

    vs_out.normal = normalize(vec3(left - right, uTexelSize, down - up));
    vs_out.normal += normalize(vec3(up_left - down_right, uTexelSize, down_left - up_right));

    vs_out.normal = normalize(vs_out.normal);

    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    vs_out.normal = normalize(normalMatrix * vs_out.normal);

    if (abs(dot(vs_out.normal, vec3(0, 1, 0))) < 0.999) {
        vs_out.tangent = normalize(cross(vs_out.normal, vec3(0, 1, 0)));
    } else {
        vs_out.tangent = normalize(cross(vs_out.normal, vec3(1, 0, 0)));
    }

    vs_out.bitangent = normalize(cross(vs_out.normal, vs_out.tangent));
    vs_out.tangent_space = mat3(vs_out.tangent, vs_out.bitangent, vs_out.normal);

    mat3 tbn = mat3(vs_out.tangent, vs_out.bitangent, vs_out.normal);
    //    vs_out.normal = tbn * vs_out.normal;
}

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

    float height = texture(height_map, tex_coord).x * terrain_height - (terrain_height / 3.0f);

    //    calculate_normal_1(tex_coord);
    //    calculate_normal_2(tex_coord);

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
    //    vec4 normal = normalize(vec4(cross(v_vector.xyz, u_vector.xyz), 0));

    // bilinearly interpolate model coodinates across patch
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p_i = (p1 - p0) * v + p0;
    vec4 p = p_i + vec4(0, height, 0, 0);

    calculate_normal_3(tex_coord);
    //    calculate_tangent_martrix(tex_coord);

    // perform the MV (Model-View) transformation.
    gl_Position = view * model * p;
}