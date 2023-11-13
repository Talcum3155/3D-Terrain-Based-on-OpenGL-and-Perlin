//
// Created by Tarowy on 2023-11-13.
//

#ifndef INC_3DPERLINMAP_GLFW_TOOL_H
#define INC_3DPERLINMAP_GLFW_TOOL_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace utilities {
    GLFWwindow *
    init_window(const char *window_name, int width = 1920, int height = 1080);

    void
    framebuffer_size_callback(GLFWwindow*,int width = 1920,int height = 1080);

    void
    process_input(GLFWwindow*);
}

#endif //INC_3DPERLINMAP_GLFW_TOOL_H
