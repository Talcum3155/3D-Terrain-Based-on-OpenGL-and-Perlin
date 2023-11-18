//
// Created by Tarowy on 2023-11-13.
//

#ifndef INC_3DPERLINMAP_GLFW_TOOL_H
#define INC_3DPERLINMAP_GLFW_TOOL_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <functional>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "camera.h"

namespace utilities {

    using void_callback = std::function<void()>;

    GLFWwindow *
    init_window(const char *window_name, int width = 1920, int height = 1080);

    GLFWwindow *
    init_window(const char *window_name, camera &cam, int width = 1920, int height = 1080);

    void
    framebuffer_size_callback(GLFWwindow *, int width = 1920, int height = 1080);

    void
    process_input(GLFWwindow *window);

    void
    process_input(GLFWwindow *window, camera &cam, float delta_time);

    void
    mouse_callback(GLFWwindow *window, double x_pos_in, double y_pos_in, camera &cam);

    void
    scroll_callback(GLFWwindow *window, double, double y_offset, camera &cam);

    unsigned int
    load_texture(std::string &&absolute_path, std::string &&texture_name);

    unsigned int
    load_texture(std::string &&absolute_path, std::string &&texture_name, int &width, int &height);

    void
    create_im_gui_context(GLFWwindow *window, std::string &&glsl_version);

    void
    config_im_gui_loop(const char* gui_name, void_callback &callback);

    void
    process_input(GLFWwindow *window, camera &cam, float delta_time, float keyboard_cool_down);
}

#endif //INC_3DPERLINMAP_GLFW_TOOL_H
