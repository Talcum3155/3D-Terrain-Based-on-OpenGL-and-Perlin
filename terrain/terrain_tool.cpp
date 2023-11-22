//
// Created by Tarowy on 2023-11-18.
//

#include "terrain_tool.h"

namespace terrain {

    /**
     * Generate the vertices of the panel tessellated by patch numbers
     * @param map_width width of height map
     * @param map_height height of height map
     * @param patch_numbers patch numbers
     * @param vertices target vector
     */
    void
    generate_terrain_vertices(const int map_width, const int map_height,
                              const int patch_numbers, std::vector<float> &vertices, float u_offset, float v_offset) {

        auto map_width_f = static_cast<float>(map_width);
        auto map_height_f = static_cast<float>(map_height);

        // compute the coordinates of every corner of each panel

        // compute the reciprocal once to minimize performance impact
        const float patch_reciprocal = 1.0f / static_cast<float>(patch_numbers);

        // divide the width and height
        float width_offset_factor = static_cast<float>(map_width) * patch_reciprocal;
        float height_offset_factor = static_cast<float>(map_height) * patch_reciprocal;

        for (auto i = 0; i <= patch_numbers - 1; ++i) {
            auto x = static_cast<float>(i);

            for (auto j = 0; j <= patch_numbers - 1; ++j) {

                auto z = static_cast<float>(j);

                // coordinates of the left lower corner of the panel
                vertices.push_back(-map_width_f * 0.5f + x * width_offset_factor);
                vertices.push_back(0.0f);
                vertices.push_back(-map_height_f * 0.5f + z * height_offset_factor);
                vertices.push_back(x * patch_reciprocal + u_offset);
                vertices.push_back(z * patch_reciprocal + v_offset);

                // coordinates of the right lower corner of the panel
                vertices.push_back(-map_width_f * 0.5f + (x + 1) * width_offset_factor);
                vertices.push_back(0.0f);
                vertices.push_back(-map_height_f * 0.5f + z * height_offset_factor);
                vertices.push_back((x + 1) * patch_reciprocal + u_offset);
                vertices.push_back(z * patch_reciprocal + v_offset);

                // coordinates of the left upper corner of the panel
                vertices.push_back(-map_width_f * 0.5f + x * width_offset_factor);
                vertices.push_back(0.0f);
                vertices.push_back(-map_height_f * 0.5f + (z + 1) * height_offset_factor);
                vertices.push_back(x * patch_reciprocal + u_offset);
                vertices.push_back((z + 1) * patch_reciprocal + v_offset);

                // coordinates of the right upper corner of the panel
                vertices.push_back(-map_width_f * 0.5f + (x + 1) * width_offset_factor);
                vertices.push_back(0.0f);
                vertices.push_back(-map_height_f * 0.5f + (z + 1) * height_offset_factor);
                vertices.push_back((x + 1) * patch_reciprocal + u_offset);
                vertices.push_back((z + 1) * patch_reciprocal + v_offset);
            }
        }
    }

    /**
     * load height data from height map as a texture
     * @param map_width
     * @param map_height
     * @param height_data noise height map data
     * @return texture id
     */
    unsigned int
    load_height_map(const int &map_width, const int &map_height, std::vector<float> &height_data) {
        unsigned int texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        // to support non-power-of-two heightmap textures
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // use GL_RED format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, map_width, map_height, 0, GL_RED, GL_FLOAT, height_data.data());

        // generate mipmap for texture
        glGenerateMipmap(GL_TEXTURE_2D);

        // set texture wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        return texture_id;
    }


    /**
     * Generate perlin noise map
     * @param height_map target height map
     * @param perlin perlin instance
     * @param map_width width of height map
     * @param map_height height of height map
     * @param scale used to scale sample point
     * @param layer_count layer counts
     * @param lacunarity used to affect the whole sample point
     * @param layer_lacunarity used to affect the lacunarity per layer
     * @param layer_amplitude used to affect the amplitude per layer
     * @param x_offset x sample offset
     * @param y_offset y sample offset
     */
    void
    get_height_map(std::vector<float> &height_map, siv::PerlinNoise &perlin, const int &map_width,
                   const int &map_height, float scale, int layer_count, float lacunarity, float layer_lacunarity,
                   float layer_amplitude,
                   float x_offset, float y_offset) {

        float max_possible_height = 0.0f;
        float amplitude = 1.0f;

        for (int i = 0; i < layer_count; ++i) {
            // accumulate height value layer by layer
            max_possible_height += amplitude;
            // decrease the height layer by layer.
            amplitude *= layer_amplitude;
        }

        float x_perlin_offset = x_offset * static_cast<float>(map_width);
        float y_perlin_offset = y_offset * static_cast<float>(map_height);

        for (int x = 0; x < map_width; ++x) {
            for (int y = 0; y < map_height; ++y) {
                float current_pixel_height = 0.0f;

                float current_layer_lacunarity = 1.0f;
                float current_layer_amplitude = 1.0f;

                float sample_x =
                        (static_cast<float>(x) + x_perlin_offset) * lacunarity * scale;
                float sample_y =
                        (static_cast<float>(y) + y_perlin_offset) * lacunarity * scale;

                for (int i = 0; i < layer_count; ++i) {
                    sample_x *= current_layer_lacunarity;
                    sample_y *= current_layer_lacunarity;

                    // sample perlin
                    current_pixel_height += static_cast<float>(perlin.noise2D_01(sample_x, sample_y) *
                                                               current_layer_amplitude);
                    current_layer_lacunarity *= layer_lacunarity;
                    current_layer_amplitude *= layer_amplitude;
                }

                current_pixel_height /= max_possible_height;
                height_map[x + y * map_height] = current_pixel_height;
            }
        }
    }

    unsigned int
    create_terrain(std::vector<float>& vertices){
        unsigned int terrain_vao, terrain_vbo;
        glGenVertexArrays(1, &terrain_vao);
        glBindVertexArray(terrain_vao);

        glGenBuffers(1, &terrain_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, terrain_vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float) * vertices.size()), &vertices[0],
                     GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        // texCoord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        return terrain_vao;
    }
}