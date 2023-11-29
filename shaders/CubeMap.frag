#version 460 core

in vec3 world_pos;
out vec4 FragColor;

uniform sampler2D equirectangular_map;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 sample_spherical_map(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}


void main() {
    // transform position vector to uv coord
    vec2 uv = sample_spherical_map(normalize(world_pos));
    // according to position vector to sample cube map
    vec3 color = texture(equirectangular_map, uv).rgb;

    FragColor = vec4(color, 1.0);
}