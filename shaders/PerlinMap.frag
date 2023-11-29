#version 460 core

#define MAX_TEXTURES 5

struct terrain_material {
    sampler2D diff[MAX_TEXTURES];
    sampler2D norm[MAX_TEXTURES];
    sampler2D ao[MAX_TEXTURES];
    sampler2D disp[MAX_TEXTURES];
    float height[MAX_TEXTURES + 1];

    float triplanar_scale;
    int triplanar_sharpness;
};

struct light_data {
    vec3 light_pos;
    vec3 view_pos;
    vec3 light_color;

    float ambient_strength;
    float specular_strength;
    float specular_pow;
};

in terrain_data {
    float height;
    float height_01;
    vec2 tex_coord;
    vec3 frag_pos;

    vec3 w_normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 blended_normal;
} data;

uniform terrain_material material;
uniform light_data light;

uniform vec3 local_pos;

uniform bool enable_light;
uniform bool enable_texture;
uniform bool enable_tangent;
uniform bool use_whiteout;
uniform bool gamma_correction;

out vec4 FragColor;

in vec3 weights;

float lower_bound;
float upper_bound;
int texture_upper_index;
int texture_lower_index;

vec3 blinn_phong_lighting(vec3 normal, vec3 diff, float ao) {
    // ambient
    vec3 ambient = light.ambient_strength * diff * light.light_color * ao;

    // diffuse
    vec3 light_dir = normalize(light.light_pos - data.frag_pos);
    float diff_factor = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * diff_factor * light.light_color;

    // specular

    return vec3(ambient + diffuse);
}

vec4 get_diff(vec2 tex) {
    // the percentage of the height value within the current height range.
    //    float percentage = (data.height_01 - lower_bound) / (upper_bound - lower_bound);

    vec4 base_color = texture2D(material.diff[texture_lower_index], tex);
    vec4 next_color = texture2D(material.diff[texture_upper_index], tex);
    return mix(base_color, next_color, smoothstep(lower_bound, upper_bound, data.height_01));
}

float get_ao(vec2 tex) {
    vec4 base_color = texture2D(material.ao[texture_lower_index], tex);
    vec4 next_color = texture2D(material.ao[texture_upper_index], tex);
    return mix(base_color, next_color, smoothstep(lower_bound, upper_bound, data.height_01)).r;
}

// triplanar to sample diff texture
vec4 get_diff_triplanar() {
    vec4 x_color = texture2D(material.diff[texture_lower_index], data.frag_pos.yz * material.triplanar_scale);
    vec4 y_color = texture2D(material.diff[texture_lower_index], data.frag_pos.xz * material.triplanar_scale);
    vec4 z_color = texture2D(material.diff[texture_lower_index], data.frag_pos.xy * material.triplanar_scale);

    vec4 base_color = x_color * weights.x + y_color * weights.y + z_color * weights.z;

    x_color = texture2D(material.diff[texture_upper_index], data.frag_pos.yz * material.triplanar_scale);
    y_color = texture2D(material.diff[texture_upper_index], data.frag_pos.xz * material.triplanar_scale);
    z_color = texture2D(material.diff[texture_upper_index], data.frag_pos.xy * material.triplanar_scale);

    vec4 next_color = x_color * weights.x + y_color * weights.y + z_color * weights.z;

    return mix(base_color, next_color, smoothstep(lower_bound, upper_bound, data.height_01));
}

float get_ao_triplanar(vec2 tex) {
    vec4 x_color = texture2D(material.ao[texture_lower_index], data.frag_pos.yz * material.triplanar_scale);
    vec4 y_color = texture2D(material.ao[texture_lower_index], data.frag_pos.xz * material.triplanar_scale);
    vec4 z_color = texture2D(material.ao[texture_lower_index], data.frag_pos.xy * material.triplanar_scale);

    vec4 base_color = x_color * weights.x + y_color * weights.y + z_color * weights.z;

    x_color = texture2D(material.ao[texture_upper_index], data.frag_pos.yz * material.triplanar_scale);
    y_color = texture2D(material.ao[texture_upper_index], data.frag_pos.xz * material.triplanar_scale);
    z_color = texture2D(material.ao[texture_upper_index], data.frag_pos.xy * material.triplanar_scale);

    vec4 next_color = x_color * weights.x + y_color * weights.y + z_color * weights.z;

    return mix(base_color, next_color, smoothstep(lower_bound, upper_bound, data.height_01)).r;
}

// triplanar to sample normal texture
vec4 get_normal_triplanar() {
    vec4 x_color = texture2D(material.norm[texture_lower_index], data.frag_pos.yz * material.triplanar_scale);
    vec4 y_color = texture2D(material.norm[texture_lower_index], data.frag_pos.xz * material.triplanar_scale);
    vec4 z_color = texture2D(material.norm[texture_lower_index], data.frag_pos.xy * material.triplanar_scale);

    vec4 base_normal = (x_color * weights.x + y_color * weights.y + z_color * weights.z);

    x_color = texture2D(material.norm[texture_upper_index], data.frag_pos.yz * material.triplanar_scale);
    y_color = texture2D(material.norm[texture_upper_index], data.frag_pos.xz * material.triplanar_scale);
    z_color = texture2D(material.norm[texture_upper_index], data.frag_pos.xy * material.triplanar_scale);

    vec4 next_normal = (x_color * weights.x + y_color * weights.y + z_color * weights.z);

    base_normal = base_normal * 2 - 1;
    next_normal = next_normal * 2 - 1;

    if (use_whiteout) {
        return vec4(normalize(vec3(base_normal.xy + next_normal.xy, base_normal.z * next_normal.z)), 1.0f);
    }

    return normalize(base_normal + next_normal);
}

void get_tex_data() {
    texture_upper_index = 0;

    for (int i = 0; i < MAX_TEXTURES + 1; i++) {
        if (data.height_01 < material.height[i]) {
            texture_upper_index = i;
            break;
        }
    }

    texture_upper_index = clamp(texture_upper_index, 0, MAX_TEXTURES - 1);
    texture_lower_index = clamp(texture_upper_index - 1, 0, MAX_TEXTURES - 1);

    lower_bound = material.height[texture_lower_index];
    upper_bound = material.height[texture_upper_index];
}

//void compute_normal_weight() {
//    weights = abs(data.w_normal);
//    weights = vec3(pow(weights.x, material.triplanar_sharpness),
//                   pow(weights.y, material.triplanar_sharpness),
//                   pow(weights.z, material.triplanar_sharpness));
//
//    weights = weights / (weights.x + weights.y + weights.z);
//}


void main()
{
    get_tex_data();
    //    compute_normal_weight();

    mat3 tbn = mat3(data.tangent, data.bitangent, data.w_normal);

    vec3 color = vec3(data.height_01, data.height_01, data.height_01);
    float ao = 1.0f;
    vec3 tex_noraml = data.w_normal;

    if (enable_texture) {
        //        color = get_diff(data.tex_coord).xyz;
        color = get_diff_triplanar().xyz;
        ao = get_ao_triplanar(data.tex_coord);
    }

    if (enable_tangent) {
        tex_noraml = tbn * get_normal_triplanar().xyz;
    }

    if (enable_light) {

        color = blinn_phong_lighting
        (
        // transform normal of normal map from tangent space to world space
            tex_noraml,
            color.xyz,
            ao
        );
    }

    if (gamma_correction) {
        color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
    }

    //    FragColor = vec4(data.blended_normal, 1.0f);
    FragColor = vec4(color, 1.0f);
}