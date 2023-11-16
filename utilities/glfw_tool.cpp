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
     * @return window
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
     * create a window with camera
     * @param window_name window name
     * @param cam camera instance
     * @param width window width
     * @param height window height
     * @return window
     */
    GLFWwindow *
    init_window(const char *window_name, camera &cam, int width, int height) {
        GLFWwindow *window = init_window(window_name, width, height);

        // capture mouse
        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);

        // store pointer to window
        glfwSetWindowUserPointer(window, &cam);

        // call the function when the mouse moves
        glfwSetCursorPosCallback(window, [](GLFWwindow *target_window, double x_pos_in, double y_pos_in) {
            // got pointer from window and cast it to camera
            auto *got_camera = static_cast<camera *>(glfwGetWindowUserPointer(target_window));
            mouse_callback(target_window, x_pos_in, y_pos_in, *got_camera);
        });
        //call the function when the mouse scrolls
        glfwSetScrollCallback(window, [](GLFWwindow *target_window, double, double y_offset) {
            auto *got_camera = static_cast<camera *>(glfwGetWindowUserPointer(target_window));
            scroll_callback(target_window, 0.0f, y_offset, *got_camera);
        });

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
    process_input(GLFWwindow *window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
    }

    /**
     * Get keyboard input
     * @param window target window
     */
    void
    process_input(GLFWwindow *window, camera &cam, float delta_time) {
        process_input(window);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cam.process_key_input(FORWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cam.process_key_input(BACKWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cam.process_key_input(LEFT, delta_time);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cam.process_key_input(RIGHT, delta_time);
    }

    void
    mouse_callback(GLFWwindow *window, double x_pos_in, double y_pos_in, camera &cam) {
        glm::vec2 pos(static_cast<float>(x_pos_in), static_cast<float>(y_pos_in));

        if (cam.first_move) {
            cam.last_pos.x = pos.x;
            cam.last_pos.y = pos.y;
            cam.first_move = false;
        }

        float x_offset = pos.x - cam.last_pos.x;
        // reversed since y-coordinates go from bottom to top
        float y_offset = cam.last_pos.y - pos.y;

        cam.last_pos.x = pos.x;
        cam.last_pos.y = pos.y;

        cam.process_mouse_movement(x_offset, y_offset, true);
    }

    void
    scroll_callback(GLFWwindow *window, double, double y_offset, camera &cam) {
        cam.process_mouse_scroll(static_cast<float>(y_offset));
    }
}