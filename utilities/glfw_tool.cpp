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
        glfwSwapInterval(1); // Enable vsync

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
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
     * Get keyboard input with camera
     * @param window target window
     * @param cam target camera
     * @param delta_time delta time between frames
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

    /**
     * Get keyboard input with camera and gui
     * @param window target window
     * @param cam target camera
     * @param delta_time delta time between frames
     * @param keyboard_cool_down
     */
    void
    process_input(GLFWwindow *window, camera &cam, float delta_time, float keyboard_cool_down) {

        // check whether the key that switches the GUI
        // is triggered within the time of cool down
        static float cool_down;

        process_input(window);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cam.process_key_input(FORWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cam.process_key_input(BACKWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cam.process_key_input(LEFT, delta_time);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cam.process_key_input(RIGHT, delta_time);

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && cool_down <= 0.0f) {
            if (cam.enable_mouse_movement) {
                cam.enable_mouse_movement = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                cam.enable_mouse_movement = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            cool_down = keyboard_cool_down;
        }

        cool_down -= delta_time;
    }

    /**
     * Get mouse current position
     * @param window window instance
     * @param x_pos_in current x value
     * @param y_pos_in current y value
     * @param cam camera instance
     */
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

    /**
     * Get mouse scroll offset
     * @param window window instance
     * @param y_offset mouse y offset
     * @param cam camera instance
     */
    void
    scroll_callback(GLFWwindow *window, double, double y_offset, camera &cam) {
        cam.process_mouse_scroll(static_cast<float>(y_offset));
    }

    /**
     * Load texture data from path
     * @param absolute_path path prefix
     * @param texture_name texture full name
     * @return texture id
     */
    unsigned int
    load_texture(std::string &&absolute_path, std::string &&texture_name) {
        int dummy_width, dummy_height;
        return load_texture(std::forward<std::string>(absolute_path),
                            std::forward<std::string>(texture_name),
                            dummy_width, dummy_height); // discard height and width
    }

    unsigned int
    load_texture(std::string &&absolute_path, std::string &&texture_name, int &width, int &height) {
        unsigned int texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        int nrComponents;
        unsigned char *data = stbi_load(std::string(absolute_path + texture_name).c_str(), &width, &height,
                                        &nrComponents, 0);
        if (data) {
            GLenum format = 0;
            switch (nrComponents) {
                case 1:
                    format = GL_RED;
                    break;
                case 3:
                    format = GL_RGB;
                    break;
                case 4:
                    format = GL_RGBA;
                    break;
                default:
                    format = GL_RGB;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);
            // generate mipmap for texture
            glGenerateMipmap(GL_TEXTURE_2D);

            // set texture wrapping
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);

        } else {
            stbi_image_free(data);
            throw std::runtime_error("Texture failed to load at path: " + absolute_path + texture_name);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        return texture_id;
    }

    void
    create_im_gui_context(GLFWwindow *window, std::string &&glsl_version) {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());
    }

    void
    config_im_gui_loop(const char *gui_name, void_callback &callback) {
        {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin(gui_name);
            callback();
            ImGui::End();
        }
    }

    void
    render_im_gui(){
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}