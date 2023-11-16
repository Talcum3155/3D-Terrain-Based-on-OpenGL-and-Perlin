#version 460 core
layout (location = 0) in vec3 aPos; // vertexs model position
layout (location = 1) in vec2 aColor;

out vec2 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    vertexColor = aColor;
}