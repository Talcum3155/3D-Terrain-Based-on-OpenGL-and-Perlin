#version 460 core

#define MAX_TEXTURES 5

struct terrain_material {
    sampler2D diff[MAX_TEXTURES];
    sampler2D norm[MAX_TEXTURES];
    sampler2D arm[MAX_TEXTURES];
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

// IBL
uniform samplerCube irradiance_map;
uniform samplerCube prefilter_map;
uniform sampler2D brdf_lut;

uniform bool enable_tangent;
uniform bool use_whiteout;
uniform bool gamma_correction;
uniform int texture_mode;
uniform int light_mode;

out vec4 FragColor;

in vec3 weights;

float lower_bound;
float upper_bound;
int texture_upper_index;
int texture_lower_index;

const float PI = 3.14159265359;

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

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal
// mapping the usual way for performance anyways; I do plan make a note of this
// technique somewhere later in the normal mapping tutorial.
//vec3 getNormalFromMap()
//{
//    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
//
//    vec3 Q1  = dFdx(WorldPos);
//    vec3 Q2  = dFdy(WorldPos);
//    vec2 st1 = dFdx(TexCoords);
//    vec2 st2 = dFdy(TexCoords);
//
//    vec3 N   = normalize(Normal);
//    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
//    vec3 B  = -normalize(cross(N, T));
//    mat3 TBN = mat3(T, B, N);
//
//    return normalize(TBN * tangentNormal);
//}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 pbr_lighting(vec3 N, vec3 albedo, float ao, float roughness, float metallic) {

    // input lighting data
    vec3 view_dir = normalize(light.view_pos - data.frag_pos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    //-----------------------------------------------------------------------------------------------------------------
    //------------------------------------------compute direct light--------------------------------------------------
    // reflectance equation
    vec3 Lo = vec3(0.0);

    // calculate per-light radiance
    vec3 light_dir = normalize(light.light_pos - data.frag_pos);
    vec3 half_dir = normalize(view_dir + light_dir);

    // compute light attenuation
    //    float distance = length(light_data.light_pos - data.frag_pos);
    //    float attenuation = 1.0 / (distance * distance);
    //    vec3 radiance = light_data.light_color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, half_dir, roughness);
    float G = GeometrySmith(N, view_dir, light_dir, roughness);
    vec3 Frenel = fresnelSchlick(max(dot(half_dir, view_dir), 0.0), F0);

    vec3 numerator = NDF * G * Frenel;
    float denominator = 4.0 * max(dot(N, view_dir), 0.0) * max(dot(N, light_dir), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular_direct_light = numerator / denominator;

    // kS is equal to Fresnel
    vec3 kS_direct_light = Frenel;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD_direct_light = vec3(1.0) - kS_direct_light;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD_direct_light *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, light_dir), 0.0);

    // add to outgoing radiance Lo
    // note that already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    Lo += (kD_direct_light * albedo / PI + specular_direct_light) * NdotL;

    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, view_dir), 0.0), F0, roughness);

    vec3 kS_ambient_light = F;
    vec3 kD_ambient_light = 1.0 - kS_ambient_light;
    kD_ambient_light *= 1.0 - metallic;

    vec3 irradiance = texture(irradiance_map, N).rgb;
    vec3 diffuse = irradiance * albedo;
    //-----------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------------------------


    //-----------------------------------------------------------------------------------------------------------------
    //-----------------------sample prefilter and brdf texture to compute ambient light--------------------------------
    // sample both the pre-filter map and the BRDF lut and combine them together
    // as per the Split-Sum approximation to get the IBL specular part.
    vec3 relfect_dir = reflect(-view_dir, N); // reflect direction to sample prefilter map
    const float MAX_REFLECTION_LOD = 4.0; // calculate the mipmap level based on roughness
    vec3 prefilteredColor = textureLod(prefilter_map, relfect_dir, roughness * MAX_REFLECTION_LOD).rgb;

    // sample the response of the BRDF in the direction of that viewing angle
    vec2 brdf = texture(brdf_lut, vec2(max(dot(N, view_dir), 0.0), roughness)).rg;
    vec3 specular_ambient_light = prefilteredColor * (F * brdf.x + brdf.y);

    // compute ambient light partion
    vec3 ambient = (kD_ambient_light * diffuse + specular_ambient_light) * ao;
    //-----------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------------------------

    // combine ambient light and diffuse
    return ambient * light.ambient_strength + Lo;
}

vec4 get_diff(vec2 tex) {
    // the percentage of the height value within the current height range.
    //    float percentage = (data.height_01 - lower_bound) / (upper_bound - lower_bound);

    vec4 base_color = texture2D(material.diff[texture_lower_index], tex);
    vec4 next_color = texture2D(material.diff[texture_upper_index], tex);
    return mix(base_color, next_color, smoothstep(lower_bound, upper_bound, data.height_01));
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

// triplanar to sample arm texture
vec4 get_arm_triplanar(vec2 tex) {
    vec4 x_color = texture2D(material.arm[texture_lower_index], data.frag_pos.yz * material.triplanar_scale);
    vec4 y_color = texture2D(material.arm[texture_lower_index], data.frag_pos.xz * material.triplanar_scale);
    vec4 z_color = texture2D(material.arm[texture_lower_index], data.frag_pos.xy * material.triplanar_scale);

    vec4 base_color = x_color * weights.x + y_color * weights.y + z_color * weights.z;

    x_color = texture2D(material.arm[texture_upper_index], data.frag_pos.yz * material.triplanar_scale);
    y_color = texture2D(material.arm[texture_upper_index], data.frag_pos.xz * material.triplanar_scale);
    z_color = texture2D(material.arm[texture_upper_index], data.frag_pos.xy * material.triplanar_scale);

    vec4 next_color = x_color * weights.x + y_color * weights.y + z_color * weights.z;

    return mix(base_color, next_color, smoothstep(lower_bound, upper_bound, data.height_01));
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


void main()
{
    get_tex_data();

    mat3 tbn = mat3(normalize(data.tangent), normalize(data.bitangent), normalize(data.w_normal));

    vec3 color = vec3(data.height_01, data.height_01, data.height_01);
    vec3 tex_noraml = normalize(data.w_normal);

    vec3 arm = get_arm_triplanar(data.tex_coord).xyz;

    switch (texture_mode) {
        case 0:
            break;
        case 1:
            color = get_diff(data.tex_coord).xyz;
            break;
        case 2:
            color = get_diff_triplanar().xyz;
            break;
        default:
            break;
    }

    if (enable_tangent) {
        // transform normal of normal map from tangent space to world space
        tex_noraml = tbn * get_normal_triplanar().xyz;
    }

    switch (light_mode) {
        case 0:
            break;
        case 1:
            color = blinn_phong_lighting(tex_noraml, color, arm.r);
            break;
        case 2: // material properties
            vec3 albedo = pow(color, vec3(2.2));
            color = pbr_lighting(tex_noraml, color, arm.r, arm.g, arm.b);
            break;
        default:
            break;
    }

    if (gamma_correction) {
        color.rgb = pow(color.rgb / (color.rgb + vec3(1.0)), vec3(1.0 / 2.2));
    }

    //    FragColor = vec4(data.blended_normal, 1.0f);
    FragColor = vec4(color, 1.0f);
}