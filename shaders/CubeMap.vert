#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 world_pos;

uniform mat4 projection;
uniform mat4 view;

void main() {
    world_pos = aPos;
    gl_Position = projection * view * vec4(world_pos, 1.0);
}