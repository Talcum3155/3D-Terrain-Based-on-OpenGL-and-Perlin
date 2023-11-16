#version 460 core
out vec4 FragColor;

in vec2 vertexColor;

void main() {
    FragColor = vec4(vertexColor, 1, 1);
}