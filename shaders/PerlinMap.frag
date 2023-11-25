#version 460 core

#define MAX_TEXTURES 5

struct terrain_material {
    sampler2D diff[MAX_TEXTURES];
    sampler2D ao[MAX_TEXTURES];
    float height[MAX_TEXTURES + 1];
};

struct light_data {
    vec3 light_pos;
    vec3 view_pos;
    vec3 light_color;

    float ambient_strength;
    float specular_strength;
    float specular_pow;
};

struct texture_data {
    float lower_bound;
    float upper_bound;
    int texture_upper_index;
    int texture_lower_index;
};

in terrain_data {
    float height;
    float height_01;
    vec2 tex_coord;
    vec3 frag_pos;
} data;
in vec3 normal;

uniform terrain_material material;
uniform light_data light;
uniform float terrain_height;

out vec4 FragColor;

texture_data tex_data;

vec3 blinn_phong_lighting(vec3 normal, vec3 diff, float ao) {
    // ambient
    vec3 ambient = light.ambient_strength * light.light_color * ao;

    // diffuse
    vec3 light_dir = normalize(light.light_pos - data.frag_pos);
    float diff_factor = max(dot(diff, light_dir), 0.0);
    vec3 diffuse = diff * diff_factor * light.light_color;

    // specular

    return vec3(ambient + diffuse);
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
}

vec4 get_diff(vec2 tex) {
    // the percentage of the height value within the current height range.
    //    float percentage = (data.height_01 - lower_bound) / (upper_bound - lower_bound);

    //    float texture_resolution = (upper_bound - lower_bound) * terrain_height;

    vec4 base_color = texture2D(material.diff[tex_data.texture_lower_index], tex);

    vec4 next_color = texture2D(material.diff[tex_data.texture_upper_index], tex);

    return mix(base_color, next_color, smoothstep(tex_data.lower_bound, tex_data.upper_bound, data.height_01));
}

vec4 get_ao(vec2 tex) {
    vec4 base_color = texture2D(material.ao[tex_data.texture_lower_index], tex);
    vec4 next_color = texture2D(material.ao[tex_data.texture_upper_index], tex);
    return mix(base_color, next_color, smoothstep(tex_data.lower_bound, tex_data.upper_bound, data.height_01));
}

void main()
{
    get_tex_data();

    //    FragColor = vec4(h, h, h, 1.0f);
    FragColor = vec4(blinn_phong_lighting
                     (vec3(0, 0, 0),
                      get_diff(data.tex_coord).xyz,
                      get_ao(data.tex_coord).r
                     ), 1);
}