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

namespace terrain {
    void
    generate_terrain_vertices(int map_width, int map_height, int patch_numbers, std::vector<float> &vertices);

    unsigned int
    load_height_map(const int &map_width, const int &map_height, std::vector<float> &height_data);
}

#endif //INC_3DPERLINMAP_TERRAIN_TOOL_H
