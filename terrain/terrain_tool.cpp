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
                              const int patch_numbers, std::vector<float> &vertices) {

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
                vertices.push_back(x * patch_reciprocal);
                vertices.push_back(z * patch_reciprocal);

                // coordinates of the right lower corner of the panel
                vertices.push_back(-map_width_f * 0.5f + (x + 1) * width_offset_factor);
                vertices.push_back(0.0f);
                vertices.push_back(-map_height_f * 0.5f + z * height_offset_factor);
                vertices.push_back((x + 1) * patch_reciprocal);
                vertices.push_back(z * patch_reciprocal);

                // coordinates of the left upper corner of the panel
                vertices.push_back(-map_width_f * 0.5f + x * width_offset_factor);
                vertices.push_back(0.0f);
                vertices.push_back(-map_height_f * 0.5f + (z + 1) * height_offset_factor);
                vertices.push_back(x * patch_reciprocal);
                vertices.push_back((z + 1) * patch_reciprocal);

                // coordinates of the right upper corner of the panel
                vertices.push_back(-map_width_f * 0.5f + (x + 1) * width_offset_factor);
                vertices.push_back(0.0f);
                vertices.push_back(-map_height_f * 0.5f + (z + 1) * height_offset_factor);
                vertices.push_back((x + 1) * patch_reciprocal);
                vertices.push_back((z + 1) * patch_reciprocal);
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

}