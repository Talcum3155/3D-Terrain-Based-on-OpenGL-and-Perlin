#version 460 core

in float height;

out vec4 FragColor;

void main()
{
    float h = (height + 16.0f) / 64.0f;
    FragColor = vec4(h, h, h, 1.0f);
}