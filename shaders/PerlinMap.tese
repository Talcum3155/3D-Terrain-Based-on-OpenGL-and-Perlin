#version 460 core

// sepcify patch type, spacing tyep, winding order for the generated primitives
layout (quads, fractional_odd_spacing, ccw) in;

#define MAX_TEXTURES 5

out terrain_data {
    float height; // real value of height
    float height_01; // height value range (0,1)
    vec2 tex_coord;
    vec3 frag_pos;

    vec3 w_normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 blended_normal;
} data;

out texture_data {
    float lower_bound;
    float upper_bound;
    flat int texture_upper_index;
    flat int texture_lower_index;
} tex_data;

struct terrain_material {
    sampler2D diff[MAX_TEXTURES];
    sampler2D norm[MAX_TEXTURES];
    sampler2D ao[MAX_TEXTURES];
    sampler2D disp[MAX_TEXTURES];
    float height[MAX_TEXTURES + 1];

    float triplanar_scale;
    int triplanar_sharpness;
};

uniform sampler2D height_map;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float terrain_height;
uniform float y_value;
uniform float HEIGHT_SCALE;
uniform float DISP;

uniform terrain_material material;

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

void calculate_normal(vec2 tex_coord) {

    float uTexelSize = 1.0 / 256.0;
    float vTexelSize = 1.0 / 256.0;

    // Sample heights around the current texture coordinate
    float left = texture(height_map, tex_coord + vec2(-uTexelSize, 0.0)).x * HEIGHT_SCALE * 2.0 - 1.0;
    float right = texture(height_map, tex_coord + vec2(uTexelSize, 0.0)).x * HEIGHT_SCALE * 2.0 - 1.0;

    float up = texture(height_map, tex_coord + vec2(0.0, vTexelSize)).x * HEIGHT_SCALE * 2.0 - 1.0;
    float down = texture(height_map, tex_coord + vec2(0.0, -vTexelSize)).x * HEIGHT_SCALE * 2.0 - 1.0;

    // construct the normal directly based on the cross-product formula.
    data.w_normal = normalize(vec3(left - right, y_value, down - up));

    // transform normal from model space to world sapce
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    data.w_normal = normalize(normalMatrix * data.w_normal);

    // may be change the order of cross
    if (abs(dot(data.w_normal, vec3(0, 1, 0))) < 0.999) {
        data.tangent = normalize(cross(data.w_normal, vec3(0, 1, 0)));
    } else {
        data.tangent = normalize(cross(data.w_normal, vec3(1, 0, 0)));
    }

    data.bitangent = normalize(cross(data.w_normal, data.tangent));
}

void get_tex_data() {
    tex_data.texture_upper_index = 0;

    for (int i = 0; i < MAX_TEXTURES + 1; i++) {
        if (data.height_01 < material.height[i]) {
            tex_data.texture_upper_index = i;
            break;
        }
    }

    tex_data.texture_upper_index = clamp(tex_data.texture_upper_index, 0, MAX_TEXTURES - 1);
    tex_data.texture_lower_index = clamp(tex_data.texture_upper_index - 1, 0, MAX_TEXTURES - 1);

    tex_data.lower_bound = material.height[tex_data.texture_lower_index];
    tex_data.upper_bound = material.height[tex_data.texture_upper_index];

    //    tex_data.texture_upper_index = 2;
    //    tex_data.texture_lower_index = 2;
    //    tex_data.lower_bound = 0.1f;
    //    tex_data.upper_bound = 0.1f;
}

vec3 get_normal(vec2 tex) {
    return texture2D(material.norm[tex_data.texture_lower_index], tex).xyz * 2 - 1;

    vec3 base_normal = texture2D(material.norm[tex_data.texture_lower_index], tex).xyz * 2 - 1;
    vec3 next_normal = texture2D(material.norm[tex_data.texture_upper_index], tex).xyz * 2 - 1;

    // whiteout blending to compute normal
    return normalize(vec3(base_normal.xy + next_normal.xy, base_normal.z * next_normal.z));
}

vec4 get_disp(vec2 tex) {
    vec4 base_color = texture2D(material.disp[tex_data.texture_lower_index], tex);
    vec4 next_color = texture2D(material.disp[tex_data.texture_upper_index], tex);
    return mix(base_color, next_color, smoothstep(tex_data.lower_bound, tex_data.upper_bound, data.height_01));
}

void main() {
    // the coordinates of the center of gravity
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    //-----------------------------------------------------------------------------------------------------------------
    // Retrieve uv of the four texture coordinates of corners of the panel
    vec2 tex_coord_h = interpolate_tex_coord(u, v, texture_coord_h[0], texture_coord_h[1],
                                             texture_coord_h[2], texture_coord_h[3]);

    data.height_01 = texture(height_map, tex_coord_h).x;
    data.height = data.height_01 * terrain_height - (terrain_height / 3.0f);

    // Retrieve the uv of texture
    data.tex_coord = interpolate_tex_coord(u, v, texture_coord[0], texture_coord[1], texture_coord[2], texture_coord[3]);
    //-----------------------------------------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------------------------------------
    // Retrieve the four model coordinates of corners of the panel
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    // bilinearly interpolate model coodinates across patch
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    //  vec4 p = (p1 - p0) * v + p0;
    vec4 p = (p1 - p0) * v + p0 + vec4(0, data.height, 0, 0);
    //-----------------------------------------------------------------------------------------------------------------


    //-----------------------------------------------------------------------------------------------------------------
    // must do model transform !!!
    data.frag_pos = vec3(model * p);
    //-----------------------------------------------------------------------------------------------------------------

    calculate_normal(tex_coord_h);

    get_tex_data();
    data.blended_normal = get_normal(data.tex_coord);
    p = p + vec4(data.blended_normal * DISP * get_disp(data.tex_coord).r, 0);

    // perform the MVP (Model-View-Projection) transformation.
    gl_Position = projection * view * model * p;
}