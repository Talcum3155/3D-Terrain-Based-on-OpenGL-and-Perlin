#define STB_IMAGE_IMPLEMENTATION

#include "utilities/glfw_tool.h"
#include "utilities/shader_g_t.h"
#include "utilities/camera.h"
#include "terrain/terrain_tool.h"
#include "terrain/map_chunk.h"

#include <unordered_map>

void load_material_texture(std::vector<unsigned int> &diff_texture);

void load_material_texture(std::vector<unsigned int> &diff_texture, std::vector<unsigned int> &ao_textures);

void load_material_texture(std::vector<unsigned int> &diff_texture, std::vector<unsigned int> &ao_textures,
                           std::vector<unsigned int> &norm_textures);

void load_material_texture(std::vector<unsigned int> &diff_texture, std::vector<unsigned int> &ao_textures,
                           std::vector<unsigned int> &norm_textures, std::vector<unsigned int> &disp_texture);

void set_texture(std::vector<unsigned int> &textures, int &texture_index, std::string &&texture_name,
                 utilities::shader &shader_program);

const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;
const unsigned short NUM_PATCH_PTS = 4;

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

const unsigned patch_numbers = 16;

const int map_width = 256;
const int map_height = 256;

const int texture_width = map_width + 2;
const int texture_height = map_height + 2;

const int render_distance = 3;

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

    std::vector<float> vertices;

    // generate a plane to tessellation
    terrain::generate_terrain_vertices(map_width, map_height, patch_numbers, vertices,
                                       1.0f / static_cast<float>(texture_width),
                                       1.0f / static_cast<float>(texture_height));

    // configure the cube's VAO (and terrainVBO)
    unsigned int terrain_vao = terrain::create_terrain(vertices);

    siv::PerlinNoise::seed_type seed(683647643u);
    siv::PerlinNoise perlin(seed);

    std::unordered_map<std::pair<int, int>, terrain::map_chunk, terrain::pair_hash> map_data;

    float scale = 0.0014f;
    int layer_count = 10;
    float lacunarity = 1.8f;
    float layer_lacunarity = 0.6f;
    float layer_amplitude = 0.5f;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            std::vector<float> height_data(texture_width * texture_height);

//            terrain::get_height_map(height_data, perlin, texture_width, texture_height,
//                                    scale, layer_count, lacunarity, layer_lacunarity, layer_amplitude,
//                                    static_cast<float>(x), static_cast<float>(y));

            terrain::get_height_map(height_data, perlin, texture_width, texture_height,
                                    scale, layer_count, static_cast<float>(x), static_cast<float>(y));

            map_data.insert({std::pair<int, int>(x, y),
                             terrain::map_chunk(x, y,
                                                std::move(height_data),
                                                terrain::load_height_map(
                                                        texture_width, texture_height, height_data)
                             )});
        }
    }

    // Specify the number of vertices per patch
    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);

    shader_program.use();
    shader_program.set_int("height_map", 0).set_float("terrain_height", 800.0f);

    shader_program_debug.use();
    shader_program_debug.set_int("height_map", 0).set_float("terrain_height", 800.0f);

#pragma set texture to shader

    // load texture
    std::vector<unsigned int> diff_textures;
    std::vector<unsigned int> ao_textures;
    std::vector<unsigned int> norm_texture;
    std::vector<unsigned int> disp_texture;
    load_material_texture(diff_textures, ao_textures, norm_texture, disp_texture);

    int texture_index = 1;
    shader_program.use();

    set_texture(diff_textures, texture_index, "diff", shader_program);
    set_texture(ao_textures, texture_index, "ao", shader_program);
    set_texture(norm_texture, texture_index, "norm", shader_program);
    set_texture(disp_texture, texture_index, "disp", shader_program);

#pragma endregion set texture to shaders

    std::vector<float> height({0.0f, 0.2f, 0.5f, 0.7f, 0.9f, 1.0f});

    for (int i = 0; i < height.size(); ++i) {
        // set height data to shader
        std::string uniform_name("material.height[" + std::to_string(i) + "]");
        shader_program.set_float(uniform_name, height[i]);
    }

    float y_value = 0.003684f;
    float HEIGHT_SCALE = 0.358f;
    bool show_normal = false;
    bool enable_light = false;
    bool enable_texture = true;
    bool enable_tangent = false;
    float DISP = 0.1f;

    float triplanar_scale = 0.51;
    int triplanar_sharpness = 8;

    float ambient_strength = 0.1;
    float light_x = 1.0f;
    float light_y = 600.0f;
    float light_z = 1.0f;

    // callback for im_gui
    std::function<void()> gui_config_callback = [&]() {
        ImGui::Text("time = %f", glfwGetTime());
        ImGui::SliderFloat("Y: ", &y_value, 0, 0.1f,"%.6f");
        ImGui::SliderFloat("HEIGHT_SCALE: ", &HEIGHT_SCALE, 0.0f, 1.0f);
        ImGui::Checkbox("Show Normal: ", &show_normal);
        ImGui::Checkbox("Show Texture: ", &enable_texture);
        ImGui::Checkbox("Show Lighting: ", &enable_light);
        ImGui::Checkbox("Enable Tangent: ", &enable_tangent);

        ImGui::NewLine();
        ImGui::InputFloat("scale: ", &scale, 0, 0.00005f, "%.6f");
        ImGui::SliderInt("layer_count: ", &layer_count, 1, 10);
        ImGui::InputFloat("lacunarity: ", &lacunarity, 0, 0.01f);
        ImGui::InputFloat("layer_lacunarity: ", &layer_lacunarity, 0, 0.01f);
        ImGui::InputFloat("layer_amplitude: ", &layer_amplitude, 0, 0.01f);

        ImGui::SliderFloat("ambient_strength: ", &ambient_strength, 0, 1);
        ImGui::InputFloat("light_x: ", &light_x);
        ImGui::InputFloat("light_y: ", &light_y);
        ImGui::InputFloat("light_z: ", &light_z);
        ImGui::SliderFloat("DISP: ", &DISP, 0.0f, 1.0f);

        ImGui::SliderFloat("tri_scale: ", &triplanar_scale, 0.0f, 0.1f);
        ImGui::SliderInt("tri_sharpness: ", &triplanar_sharpness, 1, 8);

        if (ImGui::Button("Generate Map")) {

            for (auto &map: map_data) {
//                terrain::get_height_map(map.second.height_data, perlin, map_width, map_height,
//                                        scale, layer_count, lacunarity, layer_lacunarity, layer_amplitude,
//                                        static_cast<float >(map.second.grid_x), static_cast<float>(map.second.grid_y));

                terrain::get_height_map(map.second.height_data, perlin, texture_width, texture_height,
                                        scale, layer_count, static_cast<float >(map.second.grid_x),
                                        static_cast<float>(map.second.grid_y));

                glBindTexture(GL_TEXTURE_2D, map.second.height_map_id);
                // to support non-power-of-two heightmap textures
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // use GL_RED format
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texture_width, texture_height, 0, GL_RED, GL_FLOAT,
                             map.second.height_data.data());

                // generate mipmap for texture
                glGenerateMipmap(GL_TEXTURE_2D);
            }
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

        // render the triangle
        glBindVertexArray(terrain_vao);

        glm::mat4 projection = cam.get_projection_matrix(SCR_WIDTH, SCR_HEIGHT, 0.1f, 10000.0f);

        // camera/view transformation
        glm::mat4 view = cam.get_view_matrix();

        // calculate the model matrix for each object and pass it to shader before drawing
        glm::mat4 model = glm::mat4(1.0f);

        shader_program.use();
        shader_program
                .set_mat4("projection", projection)
                .set_mat4("view", view)
                .set_vec3("light.view_pos", cam.position)
                .set_vec3("light.light_pos", glm::vec3(light_x, light_y, light_z))
                .set_vec3("light.light_color", glm::vec3(1, 1, 1))
                .set_float("light.ambient_strength", ambient_strength)
                .set_float("y_value", y_value)
                .set_float("HEIGHT_SCALE", HEIGHT_SCALE)
                .set_float("DISP", DISP)
                .set_float("material.triplanar_scale", triplanar_scale)
                .set_int("material.triplanar_sharpness", triplanar_sharpness)
                .set_bool("enable_light", enable_light)
                .set_bool("enable_texture", enable_texture)
                .set_bool("enable_tangent", enable_tangent);

        for (auto &map: map_data) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, map.second.height_map_id);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3
                    (
                            map.second.grid_x * map_width,
                            0,
                            map.second.grid_y * map_height
                    ));
            shader_program
                    .set_mat4("model", model);

            glDrawArrays(GL_PATCHES, 0, static_cast<GLsizei>(NUM_PATCH_PTS * patch_numbers * patch_numbers));
        }

        if (show_normal) {
            shader_program_debug.use();
            shader_program_debug
                    .set_mat4("projection", projection)
                    .set_mat4("view", view)
                    .set_float("y_value", y_value)
                    .set_float("HEIGHT_SCALE", HEIGHT_SCALE);

            // calculate the model matrix for each object and pass it to shader before drawing
            for (auto &map: map_data) {
                glBindTexture(GL_TEXTURE_2D, map.second.height_map_id);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3
                        (
                                map.second.grid_x * map_width,
                                0,
                                map.second.grid_y * map_height
                        ));
                shader_program_debug
                        .set_mat4("model", model);

                glDrawArrays(GL_PATCHES, 0, static_cast<GLsizei>(NUM_PATCH_PTS * patch_numbers * patch_numbers));
            }
        }

        utilities::render_im_gui();

        // swap the color buffer
        glfwSwapBuffers(window);
        // checks if any events are triggered per frame
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &terrain_vao);
//    glDeleteBuffers(1, &terrainVBO);
    glDeleteProgram(shader_program.id);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void load_material_texture(std::vector<unsigned int> &diff_textures) {
    diff_textures.push_back(utilities::load_texture("../assets/images/coast_sand/", "coast_sand_05_diff_2k.png"));
    diff_textures.push_back(utilities::load_texture("../assets/images/leafy_grass/", "leafy_grass_diff_2k.png"));
    diff_textures.push_back(utilities::load_texture("../assets/images/forest_ground/", "forest_ground_04_diff_2k.png"));
    diff_textures.push_back(utilities::load_texture("../assets/images/rock_06/", "rock_06_diff_2k.png"));
    diff_textures.push_back(utilities::load_texture("../assets/images/snow_field/", "snow_field_aerial_col_4k.png"));
}

void load_material_texture(std::vector<unsigned int> &diff_texture, std::vector<unsigned int> &ao_textures) {
    load_material_texture(diff_texture);

    ao_textures.push_back(utilities::load_texture("../assets/images/coast_sand/", "coast_sand_05_ao_2k.png"));
    ao_textures.push_back(utilities::load_texture("../assets/images/leafy_grass/", "leafy_grass_ao_2k.png"));
    ao_textures.push_back(utilities::load_texture("../assets/images/forest_ground/", "forest_ground_04_ao_2k.png"));
    ao_textures.push_back(utilities::load_texture("../assets/images/rock_06/", "rock_06_ao_2k.png"));
    ao_textures.push_back(utilities::load_texture("../assets/images/snow_field/", "snow_field_aerial_ao_4k.png"));
}

void load_material_texture(std::vector<unsigned int> &diff_texture, std::vector<unsigned int> &ao_textures,
                           std::vector<unsigned int> &norm_textures) {
    load_material_texture(diff_texture, ao_textures);

    norm_textures.push_back(utilities::load_texture("../assets/images/coast_sand/", "coast_sand_05_nor_gl_2k.png"));
    norm_textures.push_back(utilities::load_texture("../assets/images/leafy_grass/", "leafy_grass_nor_gl_2k.png"));
    norm_textures.push_back(
            utilities::load_texture("../assets/images/forest_ground/", "forest_ground_04_nor_gl_2k.png"));
    norm_textures.push_back(utilities::load_texture("../assets/images/rock_06/", "rock_06_nor_gl_2k.png"));
    norm_textures.push_back(utilities::load_texture("../assets/images/snow_field/", "snow_field_aerial_nor_gl_4k.png"));
}

void load_material_texture(std::vector<unsigned int> &diff_texture, std::vector<unsigned int> &ao_textures,
                           std::vector<unsigned int> &norm_textures, std::vector<unsigned int> &disp_texture) {
    load_material_texture(diff_texture, ao_textures, norm_textures);

    disp_texture.push_back(utilities::load_texture("../assets/images/coast_sand/", "coast_sand_05_disp_2k.png"));
    disp_texture.push_back(utilities::load_texture("../assets/images/leafy_grass/", "leafy_grass_disp_2k.png"));
    disp_texture.push_back(
            utilities::load_texture("../assets/images/forest_ground/", "forest_ground_04_disp_2k.png"));
    disp_texture.push_back(utilities::load_texture("../assets/images/rock_06/", "rock_06_disp_2k.png"));
    disp_texture.push_back(utilities::load_texture("../assets/images/snow_field/", "snow_field_aerial_height_4k.png"));
}

void set_texture(std::vector<unsigned int> &textures, int &texture_index,
                 std::string &&texture_name, utilities::shader &shader_program) {
    for (int i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        // set texture to shader
        std::string uniform_name("material." + texture_name + "[" + std::to_string(i) + "]");
        shader_program.set_int(uniform_name, texture_index);
        texture_index++;
    }
}