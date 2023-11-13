//
// Created by Tarowy on 2023-11-13.
//

#include "glfw_tool.h"

namespace utilities {

    /**
     * Create a window
     * @param width window width
     * @param height window height
     * @param window_name window name
     */
    GLFWwindow *
    init_window(const char *window_name, int width, int height) {
        glfwInit();

        // use opengl4.6
        // so the major version is set to 4, the minor version is set to 6
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

        // tell GLFW want to explicitly use the core-profile
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow *window = glfwCreateWindow(width, height, window_name, nullptr, nullptr);
        if (window == nullptr) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        // make the context of window the main context on the current thread.
        glfwMakeContextCurrent(window);

        //  load the address of the OpenGL function pointers which is OS-specific.
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            glfwTerminate();
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // The first two parameters set the location of the lower left corner of the window
        // map from the range (-1 to 1) to (0, width) and (0, height).
        glViewport(0, 0, width, height);

        // register FramebufferSize functions, call the function when the window changes in size
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        return window;
    }

    /**
     * Whenever the window changes in size,
     * GLFW calls this function and fills in the proper arguments
     * @param window target window
     * @param width window width
     * @param height window height
     */
    void
    framebuffer_size_callback(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    /**
     * Get keyboard input
     * @param window target window
     */
    void
    process_input(GLFWwindow* window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
    }


}