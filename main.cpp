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

std::tuple<unsigned int, unsigned int>
pbr_pre_process(utilities::shader &cube_map_shader, unsigned int &env_cube_map_id,
                utilities::shader &irradiance_shader, unsigned int &irradiance_map_id,
                utilities::shader &prefilter_shader, unsigned int &prefilter_map_id,
                utilities::shader &brdf_shader, unsigned int &brdf_lut_map_id);

unsigned int
render_sky_box(utilities::shader &cube_map_shader, unsigned int &hdr_texture,
               unsigned int &capture_fbo, unsigned int &capture_rbo, std::vector<glm::mat4> &capture_views);

unsigned int
render_irradiance_map(utilities::shader &irradiance_shader, unsigned int &env_cube_map_id,
                      unsigned int &capture_fbo, unsigned int &capture_rbo, std::vector<glm::mat4> &capture_views);

unsigned int
render_prefilter_map(utilities::shader &prefilter_shader, unsigned int &env_cube_map_id,
                     unsigned int &capture_fbo, unsigned int &capture_rbo, std::vector<glm::mat4> &capture_views);

unsigned int
render_brdf_lut_map(utilities::shader &brdf_shader, unsigned int &capture_fbo, unsigned int &capture_rbo);

void render_cube();

void render_quad();

const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;
const unsigned short NUM_PATCH_PTS = 4;

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

const int cube_map_resolution = 4096;
const int irradiance_resolution = 32;
const int prefilter_resolution = 128;
const int brdf_resolution = 512;

const unsigned patch_numbers = 16;

const int map_width = 256;
const int map_height = 256;

const int texture_width = map_width + 2;
const int texture_height = map_height + 2;

const int terrain_height = 400;

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

    // seamless cube map
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);

#pragma region load shader programe
    utilities::shader_t shader_program(std::string("../shaders/"), std::string("PerlinMap.vert"),
                                       std::string("PerlinMap.frag"), std::string("PerlinMap.tesc"),
                                       std::string("PerlinMap.tese"));

    utilities::shader_g_t shader_program_debug(std::string("../shaders/"), std::string("NormalTest.vert"),
                                               std::string("NormalTest.frag"), std::string("NormalTest.tesc"),
                                               std::string("NormalTest.tese"), std::string("NormalTest.geom"));

    utilities::shader cube_map_shader(std::string("../shaders/"), std::string("CubeMap.vert"),
                                      std::string("CubeMap.frag"));

    utilities::shader background_shader(std::string("../shaders/"), std::string("BackGround.vert"),
                                        std::string("BackGround.frag"));

    utilities::shader irradiance_shader(std::string("../shaders/"), std::string("CubeMap.vert"),
                                        std::string("IrradianceConvolution.frag"));

    utilities::shader prefilter_shader(std::string("../shaders/"), std::string("CubeMap.vert"),
                                       std::string("Prefilter.frag"));

    utilities::shader brdf_lut_shader(std::string("../shaders/"), std::string("BRDF.vert"),
                                      std::string("BRDF.frag"));

#pragma endregion

#pragma region generate vertices of plane

    std::vector<float> vertices;

    // generate a plane to tessellation
    terrain::generate_terrain_vertices(map_width, map_height, patch_numbers, vertices,
                                       1.0f / static_cast<float>(texture_width),
                                       1.0f / static_cast<float>(texture_height));

    // configure the cube's VAO (and terrainVBO)
    auto [terrain_vao, terrain_vbo] = terrain::create_terrain(vertices);

#pragma endregion

#pragma region generate height data

    siv::PerlinNoise::seed_type seed(683647643u);
    siv::PerlinNoise perlin(seed);

    std::unordered_map<std::pair<int, int>, terrain::map_chunk, terrain::pair_hash> map_data;

    float scale = 0.002f;
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
#pragma endregion

    // Specify the number of vertices per patch
    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);

    shader_program.use();
    shader_program.set_int("height_map", 0).set_float("terrain_height", terrain_height);

    shader_program_debug.use();
    shader_program_debug.set_int("height_map", 0).set_float("terrain_height", terrain_height);

#pragma region hdr texture to sky box

    unsigned int env_cube_map_id, irradiance_map_id, prefilter_map_id, brdf_lut_map_id;
    auto [capture_fbo, capture_rbo] =
            pbr_pre_process(cube_map_shader, env_cube_map_id,
                            irradiance_shader, irradiance_map_id,
                            prefilter_shader, prefilter_map_id,
                            brdf_lut_shader, brdf_lut_map_id);

#pragma endregion

#pragma region set terrain texture to shader

    // load texture
    std::vector<unsigned int> diff_textures;
    std::vector<unsigned int> ao_textures;
    std::vector<unsigned int> norm_texture;
//    std::vector<unsigned int> disp_texture;
//    load_material_texture(diff_textures, ao_textures, norm_texture);

    int texture_index = 1;
    shader_program.use();

//    set_texture(diff_textures, texture_index, "diff", shader_program);
//    set_texture(ao_textures, texture_index, "ao", shader_program);
//    set_texture(norm_texture, texture_index, "norm", shader_program);
//    set_texture(disp_texture, texture_index, "disp", shader_program);

#pragma endregion set texture to shaders

#pragma region specify height range of different terrain environment

    // height range of each texture
    std::vector<float> height({0.0f, 0.2f, 0.5f, 0.7f, 0.9f, 1.0f});

    for (int i = 0; i < height.size(); ++i) {
        // set height data to shader
        std::string uniform_name("material.height[" + std::to_string(i) + "]");
        shader_program.set_float(uniform_name, height[i]);
    }

#pragma endregion

#pragma region shader option

    float y_value = 0.005184f;
    float HEIGHT_SCALE = 0.358f;
    bool show_normal = false;
    bool enable_light = false;
    bool enable_texture = true;
    bool enable_tangent = false;
    bool use_whiteout = false;
    bool gamma_correction = true;
    float DISP = 0.1f;

    float triplanar_scale = 0.05;
    int triplanar_sharpness = 8;

    float ambient_strength = 0.1;
    float light_x = 1.0f;
    float light_y = 600.0f;
    float light_z = 1.0f;

    // callback for im_gui
    std::function<void()> gui_config_callback = [&]() {
        ImGui::Text("time = %f", glfwGetTime());
        ImGui::SliderFloat("Y: ", &y_value, 0, 0.01f, "%.6f");
        ImGui::SliderFloat("HEIGHT_SCALE: ", &HEIGHT_SCALE, 0.0f, 1.0f);
        ImGui::Checkbox("Show Normal: ", &show_normal);
        ImGui::Checkbox("Show Texture: ", &enable_texture);
        ImGui::Checkbox("Show Lighting: ", &enable_light);
        ImGui::Checkbox("Enable Tangent: ", &enable_tangent);
        ImGui::Checkbox("Use Whiteout: ", &use_whiteout);
        ImGui::Checkbox("Gamma Correction: ", &gamma_correction);

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

#pragma endregion option

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (!glfwWindowShouldClose(window)) {

        utilities::config_im_gui_loop("Debug", gui_config_callback);

        // per-frame time logic
        // --------------------
        auto currentFrame = static_cast<float>(glfwGetTime());
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

#pragma region render terrain

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
                .set_bool("enable_tangent", enable_tangent)
                .set_bool("use_whiteout", use_whiteout)
                .set_bool("gamma_correction", gamma_correction);

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

#pragma endregion

#pragma region render normal of terrain

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

#pragma endregion

#pragma region render sky box

        // render skybox (render as last to prevent overdraw)
        background_shader.use();
        background_shader
                .set_mat4("projection", projection)
                .set_mat4("view", view);
        // bind texture0 to cube map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map_id);
        render_cube();

#pragma endregion

        utilities::render_im_gui();

        // swap the color buffer
        glfwSwapBuffers(window);
        // checks if any events are triggered per frame
        glfwPollEvents();
    }

#pragma region clean memory

    glDeleteVertexArrays(1, &terrain_vao);
    glDeleteBuffers(1, &terrain_vbo);
    glDeleteProgram(shader_program.id);
    glDeleteProgram(shader_program_debug.id);
    glDeleteProgram(background_shader.id);
    glDeleteProgram(irradiance_shader.id);
    glDeleteProgram(cube_map_shader.id);
    glDeleteFramebuffers(1, &capture_fbo);
    glDeleteRenderbuffers(1, &capture_rbo);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

#pragma endregion


    return 0;
}

void load_material_texture(std::vector<unsigned int> &diff_textures) {
    diff_textures.push_back(utilities::load_texture("../assets/images/coast_sand/", "coast_sand_05_diff_2k.png"));
    diff_textures.push_back(
            utilities::load_texture("../assets/images/coast_sand_rocks/", "coast_sand_rocks_02_diff_2k.png"));
    diff_textures.push_back(utilities::load_texture("../assets/images/forest_ground/", "forest_ground_04_diff_2k.png"));
    diff_textures.push_back(utilities::load_texture("../assets/images/rock_06/", "rock_06_diff_2k.png"));
    diff_textures.push_back(utilities::load_texture("../assets/images/snow_field/", "snow_field_aerial_col_4k.png"));
}

void load_material_texture(std::vector<unsigned int> &diff_texture, std::vector<unsigned int> &ao_textures) {
    load_material_texture(diff_texture);

    ao_textures.push_back(utilities::load_texture("../assets/images/coast_sand/", "coast_sand_05_ao_2k.png"));
    ao_textures.push_back(
            utilities::load_texture("../assets/images/coast_sand_rocks/", "coast_sand_rocks_02_ao_2k.png"));
    ao_textures.push_back(utilities::load_texture("../assets/images/forest_ground/", "forest_ground_04_ao_2k.png"));
    ao_textures.push_back(utilities::load_texture("../assets/images/rock_06/", "rock_06_ao_2k.png"));
    ao_textures.push_back(utilities::load_texture("../assets/images/snow_field/", "snow_field_aerial_ao_4k.png"));
}

void load_material_texture(std::vector<unsigned int> &diff_texture, std::vector<unsigned int> &ao_textures,
                           std::vector<unsigned int> &norm_textures) {
    load_material_texture(diff_texture, ao_textures);

    norm_textures.push_back(utilities::load_texture("../assets/images/coast_sand/", "coast_sand_05_nor_gl_2k.png"));
    norm_textures.push_back(
            utilities::load_texture("../assets/images/coast_sand_rocks/", "coast_sand_rocks_02_nor_gl_2k.png"));
    norm_textures.push_back(
            utilities::load_texture("../assets/images/forest_ground/", "forest_ground_04_nor_gl_2k.png"));
    norm_textures.push_back(utilities::load_texture("../assets/images/rock_06/", "rock_06_nor_gl_2k.png"));
    norm_textures.push_back(utilities::load_texture("../assets/images/snow_field/", "snow_field_aerial_nor_gl_4k.png"));
}

void load_material_texture(std::vector<unsigned int> &diff_texture, std::vector<unsigned int> &ao_textures,
                           std::vector<unsigned int> &norm_textures, std::vector<unsigned int> &disp_texture) {
    load_material_texture(diff_texture, ao_textures, norm_textures);

    disp_texture.push_back(utilities::load_texture("../assets/images/coast_sand/", "coast_sand_05_disp_2k.png"));
    disp_texture.push_back(
            utilities::load_texture("../assets/images/coast_sand_rocks/", "coast_sand_rocks_02_disp_2k.png"));
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

void render_cube() {
    static unsigned int cube_vao;
    static unsigned int cube_vbo;

    // initialize (if necessary)
    if (cube_vao == 0) {
        float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f  // bottom-left
        };

        glGenVertexArrays(1, &cube_vao);
        glGenBuffers(1, &cube_vbo);

        glBindVertexArray(cube_vao);

        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // render Cube
    glBindVertexArray(cube_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
void render_quad() {
    static unsigned int quad_vao;
    static unsigned int quad_vbo;

    if (quad_vao == 0) {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quad_vao);
        glGenBuffers(1, &quad_vbo);
        glBindVertexArray(quad_vao);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    }

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

std::tuple<unsigned int, unsigned int>
pbr_pre_process(utilities::shader &cube_map_shader, unsigned int &env_cube_map_id,
                utilities::shader &irradiance_shader, unsigned int &irradiance_map_id,
                utilities::shader &prefilter_shader, unsigned int &prefilter_map_id,
                utilities::shader &brdf_shader, unsigned int &brdf_lut_map_id) {

    // set framebuffer and renderbuffer for rending sky box
    unsigned int capture_fbo, capture_rbo;
    // generate framebuffer and renderbuffer
    glGenFramebuffers(1, &capture_fbo);
    glGenRenderbuffers(1, &capture_rbo);

    // load hdr texture
    stbi_set_flip_vertically_on_load(true);
    unsigned int hdr_texture = utilities::load_texture(std::string("../assets/images/"),
                                                       std::string("blue_photo_studio_4k.hdr"), true);

    // fov 90 to capture all scene
    glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    // the view matrix of every faces
    std::vector<glm::mat4> capture_views =
            {
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, -1.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                                glm::vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                                glm::vec3(0.0f, -1.0f, 0.0f))
            };

    cube_map_shader.use();
    cube_map_shader
            .set_int("environment_map", 0)
            .set_mat4("projection", capture_projection);
    env_cube_map_id = render_sky_box(cube_map_shader, hdr_texture,
                                     capture_fbo, capture_rbo, capture_views);

    irradiance_shader.use();
    irradiance_shader
            .set_int("environment_map", 0)
            .set_mat4("projection", capture_projection);
    irradiance_map_id = render_irradiance_map(irradiance_shader, env_cube_map_id,
                                              capture_fbo, capture_rbo, capture_views);

    prefilter_shader.use();
    prefilter_shader
            .set_int("environment_map", 0)
            .set_mat4("projection", capture_projection);
    prefilter_map_id = render_prefilter_map(prefilter_shader, env_cube_map_id, capture_fbo, capture_rbo, capture_views);

    brdf_shader.use();
    brdf_lut_map_id = render_brdf_lut_map(brdf_shader, capture_fbo, capture_rbo);

    return {capture_fbo, capture_rbo};
}

unsigned int
render_sky_box(utilities::shader &cube_map_shader, unsigned int &hdr_texture,
               unsigned int &capture_fbo, unsigned int &capture_rbo, std::vector<glm::mat4> &capture_views) {
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);

    // set render buffer as depth attachment
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cube_map_resolution, cube_map_resolution);
    // bind depth attachment for framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);

    // generate an empty texture as sky box
    unsigned int env_cube_map_id;
    glGenTextures(1, &env_cube_map_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_cube_map_id);

    // bind every face of texture for cube map
    for (int i = 0; i < 6; ++i) {
        // note that store each face with 16 bit floating point values
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                     cube_map_resolution, cube_map_resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // enable trilinear filtering for prefilter map stage
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // convert HDR equirectangular environment map to cube map equivalent
    cube_map_shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_texture);

    // configure the viewport to the capture dimensions.
    glViewport(0, 0, cube_map_resolution, cube_map_resolution);

    // render sky box to framer buffer
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    for (int i = 0; i < 6; ++i) {
        cube_map_shader.set_mat4("view", capture_views[i]);

        // set cube map texture as texture attachment
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_cube_map_id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_cube();
    }

    // let OpenGL generate mipmaps from first mip face (combating visible dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_cube_map_id);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return env_cube_map_id;
}

unsigned int
render_irradiance_map(utilities::shader &irradiance_shader, unsigned int &env_cube_map_id,
                      unsigned int &capture_fbo, unsigned int &capture_rbo, std::vector<glm::mat4> &capture_views) {

    unsigned int irradiance_map_id;
    glGenTextures(1, &irradiance_map_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map_id);

    // set texture type
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                     irradiance_resolution, irradiance_resolution, 0,
                     GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);

    // set render buffer as depth attachment
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradiance_resolution, irradiance_resolution);
    // bind depth attachment for framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);

    irradiance_shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_cube_map_id);

    // configure the viewport to the capture dimensions.
    glViewport(0, 0, irradiance_resolution, irradiance_resolution);

    // render each face of irradiance cube map
    for (unsigned int i = 0; i < 6; ++i) {
        irradiance_shader.set_mat4("view", capture_views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map_id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_cube();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return irradiance_map_id;
}

unsigned int
render_prefilter_map(utilities::shader &prefilter_shader, unsigned int &env_cube_map_id,
                     unsigned int &capture_fbo, unsigned int &capture_rbo, std::vector<glm::mat4> &capture_views) {
    unsigned int prefilter_map_id;
    glGenTextures(1, &prefilter_map_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map_id);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                     prefilter_resolution, prefilter_resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // be sure to set minification filter to mip_linear
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // generate mipmaps for the cube map so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_cube_map_id);

    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    int maxMipLevels = 5;
    // render prefilter map for each mipmap level
    for (int mip = 0; mip < maxMipLevels; ++mip) {

        // reduce mipmap resolution with each iteration
        auto mip_width = static_cast<GLsizei>(128 * std::pow(0.5, mip));
        auto mip_height = static_cast<GLsizei>(128 * std::pow(0.5, mip));

        // Reset the resolution of the depth attachment to match mipmap level
        glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height);
        glViewport(0, 0, mip_width, mip_height);

        // calculate the roughness for each mipmap level
        auto roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        prefilter_shader.set_float("roughness", roughness);

        // render each face once
        for (unsigned int i = 0; i < 6; ++i) {
            prefilter_shader.set_mat4("view", capture_views[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_map_id, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            render_cube();
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return prefilter_map_id;
}

unsigned int
render_brdf_lut_map(utilities::shader &brdf_shader, unsigned int &capture_fbo, unsigned int &capture_rbo) {
    unsigned int brdf_lut_map_id;
    glGenTextures(1, &brdf_lut_map_id);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdf_lut_map_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdf_resolution, brdf_resolution, 0, GL_RG, GL_FLOAT, nullptr);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, brdf_resolution, brdf_resolution);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut_map_id, 0);

    glViewport(0, 0, brdf_resolution, brdf_resolution);

    brdf_shader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_quad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return brdf_lut_map_id;
}
