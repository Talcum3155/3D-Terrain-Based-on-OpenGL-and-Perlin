#include "utilities/glfw_tool.h"

const int width = 800;
const int height = 600;

int main() {

    GLFWwindow *window;
    try {
        window = utilities::init_window("3D Perlin Map", width, height);
    } catch (std::runtime_error &error) {
        std::cout << error.what() << std::endl;
        return -1;
    } catch (std::exception &error) {
        std::cout << error.what() << std::endl;
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {

        utilities::process_input(window);

        // swap the color buffer
        glfwSwapBuffers(window);

        // checks if any events are triggered per frame
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}