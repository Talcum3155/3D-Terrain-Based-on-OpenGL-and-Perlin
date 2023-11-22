//
// Created by Tarowy on 2023-11-18.
//

#ifndef INC_3DPERLINMAP_TERRAIN_TOOL_H
#define INC_3DPERLINMAP_TERRAIN_TOOL_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <stb_image_write.h>
#include <PerlinNoise.hpp>
#include <random>

namespace terrain {
    void
    generate_terrain_vertices(int map_width, int map_height, int patch_numbers, std::vector<float> &vertices,
                              float u_offset = 0, float v_offset = 0);

    unsigned int
    load_height_map(const int &map_width, const int &map_height, std::vector<float> &height_data);

    void
    get_height_map(std::vector<float> &height_map, siv::PerlinNoise &perlin, const int &map_width,
                   const int &map_height, float scale, int layer_count,float lacunarity, float layer_lacunarity, float layer_amplitude,
                   float x_offset, float y_offset);

    unsigned int
    create_terrain(std::vector<float>& vertices);
}

#endif //INC_3DPERLINMAP_TERRAIN_TOOL_H
