#version 460 core

in float height;

out vec4 FragColor;

void main()
{
    float h = (height + 200.0f) / 500.0f;
    FragColor = vec4(h, h, h, 1.0f);
}