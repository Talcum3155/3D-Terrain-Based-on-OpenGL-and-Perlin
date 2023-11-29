#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 world_pos;

void main()
{
    world_pos = aPos;

    // discard translate operation of view
    mat4 rot_view = mat4(mat3(view));
    vec4 clipPos = projection * rot_view * vec4(world_pos, 1.0);

    // use xyww to make the depth keep max dpeth value: 1.0
    gl_Position = clipPos.xyww;
}