#define STB_IMAGE_IMPLEMENTATION

#include "utilities/glfw_tool.h"
#include "utilities/shader_g_t.h"
#include "utilities/camera.h"
#include "terrain/terrain_tool.h"

const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;
const unsigned short NUM_PATCH_PTS = 4;

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

int main() {

    utilities::camera cam(glm::vec2(SCR_WIDTH * 0.5f, SCR_HEIGHT * 0.5f), glm::vec3(0.0f, 0.0f, 3.0f));

    GLFWwindow *window;
    try {
        window = utilities::init_window("3D Perlin Map", cam, SCR_WIDTH, SCR_HEIGHT);
    } catch (std::runtime_error &error) {
        std::cout << error.what() << std::endl;
        return -1;
    } catch (std::exception &error) {
        std::cout << error.what() << std::endl;
        return -1;
    }

    utilities::create_im_gui_context(window, "#version 460");

    GLint maxTessLevel;
    glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
    std::cout << "Max available tess level: " << maxTessLevel << std::endl;

    // enable depth test
    glEnable(GL_DEPTH_TEST);

    utilities::shader_t shader_program(std::string("../shaders/"), std::string("PerlinMap.vert"),
                                       std::string("PerlinMap.frag"), std::string("PerlinMap.tesc"),
                                       std::string("PerlinMap.tese"));

    utilities::shader_g_t shader_program_debug(std::string("../shaders/"), std::string("NormalTest.vert"),
                                               std::string("NormalTest.frag"), std::string("NormalTest.tesc"),
                                               std::string("NormalTest.tese"), std::string("NormalTest.geom"));

    int map_width = 512;
    int map_height = 512;

    // load height map
//    unsigned int height_map_id
//            = utilities::load_texture("../assets/images/", "iceland.png", map_width, map_height);

    siv::PerlinNoise::seed_type seed = 123456u;
    siv::PerlinNoise perlin(seed);

    std::vector<float> height_data((map_width) * (map_width));
//    for (int i = 0; i < map_width * map_height * 0.5; ++i) {
//        height_data[i] = 0.0f;
//    }

    float scale = 0.0015f;
    int layer_count = 5;
    float lacunarity = 1.8f;
    float layer_lacunarity = 0.6f;
    float layer_amplitude = 0.5f;

    terrain::get_height_map(height_data, perlin, map_width, map_height,
                            scale, layer_count, lacunarity, layer_lacunarity, layer_amplitude, 1.0f, 2.0f);
    unsigned int height_map_id = terrain::load_height_map(map_width, map_height, height_data);

    std::vector<float> vertices;
    const unsigned patch_numbers = 10;

//    terrain::generate_terrain_vertices(map_width, map_height, patch_numbers, vertices, 1.0f / static_cast<float>(map_width), 1.0f / static_cast<float>(map_height));
    terrain::generate_terrain_vertices(map_width, map_height, patch_numbers, vertices);

    std::cout << "Loaded " << patch_numbers * patch_numbers << " patches of 4 control points each" << std::endl;
    std::cout << "Processing " << patch_numbers * patch_numbers * 4 << " vertices in vertex shader" << std::endl;
    std::cout << "vertices size: " << vertices.size() << std::endl;

    // configure the cube's VAO (and terrainVBO)
    unsigned int terrainVAO = terrain::create_terrain(vertices);

    // Specify the number of vertices per patch
    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);

    shader_program.use();
    shader_program.set_int("height_map", 0);

    shader_program_debug.use();
    shader_program_debug.set_int("height_map", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, height_map_id);


    float y_value = 0.1f;
    float HEIGHT_SCALE = 0.1f; // may be 2.6 is best
    bool show_normal = false;

    // callback for im_gui
    std::function<void()> gui_config_callback = [&]() {
        ImGui::Text("time = %f", glfwGetTime());
        ImGui::SliderFloat("Y: ", &y_value, 0, 0.3f);
        ImGui::SliderFloat("HEIGHT_SCALE: ", &HEIGHT_SCALE, 0.5f, 50.f);
        ImGui::Checkbox("Show Normal: ", &show_normal);

        ImGui::NewLine();
        ImGui::InputFloat("scale: ",&scale,0,0.00005f,"%.6f");
        ImGui::SliderInt("layer_count: ", &layer_count, 1, 10);
        ImGui::InputFloat("lacunarity: ", &lacunarity, 0, 0.01f);
        ImGui::InputFloat("layer_lacunarity: ", &layer_lacunarity, 0, 0.01f);
        ImGui::InputFloat("layer_amplitude: ", &layer_amplitude, 0, 0.01f);

        if (ImGui::Button("Generate Map")) {

            terrain::get_height_map(height_data, perlin, map_width, map_height,
                                    scale, layer_count, lacunarity, layer_lacunarity, layer_amplitude, 1.0f, 2.0f);

            glBindTexture(GL_TEXTURE_2D, height_map_id);
            // to support non-power-of-two heightmap textures
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            // use GL_RED format
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, map_width, map_height, 0, GL_RED, GL_FLOAT, height_data.data());

            // generate mipmap for texture
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    };

//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (!glfwWindowShouldClose(window)) {

        utilities::config_im_gui_loop("Debug", gui_config_callback);

        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        utilities::process_input(window, cam, deltaTime, 0.5f);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = cam.get_projection_matrix(SCR_WIDTH, SCR_HEIGHT, 0.1f, 10000.0f);

        // camera/view transformation
        glm::mat4 view = cam.get_view_matrix();

        // calculate the model matrix for each object and pass it to shader before drawing
        glm::mat4 model = glm::mat4(1.0f);

        shader_program.use();
        shader_program
                .set_mat4("projection", projection)
                .set_mat4("view", view)
                .set_mat4("model", model);

        // render the triangle
        glBindVertexArray(terrainVAO);

        glDrawArrays(GL_PATCHES, 0, static_cast<GLsizei>(NUM_PATCH_PTS * patch_numbers * patch_numbers));

        if (show_normal) {
            shader_program_debug.use();
            shader_program_debug
                    .set_mat4("projection", projection)
                    .set_mat4("view", view)
                    .set_mat4("model", model)
                    .set_float("y_value", y_value)
                    .set_float("HEIGHT_SCALE", HEIGHT_SCALE);

            glDrawArrays(GL_PATCHES, 0, static_cast<GLsizei>(NUM_PATCH_PTS * patch_numbers * patch_numbers));
        }

        utilities::render_im_gui();

        // swap the color buffer
        glfwSwapBuffers(window);
        // checks if any events are triggered per frame
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &terrainVAO);
//    glDeleteBuffers(1, &terrainVBO);
    glDeleteProgram(shader_program.id);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}