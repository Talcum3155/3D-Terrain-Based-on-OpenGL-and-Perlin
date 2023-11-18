//
// Created by Tarowy on 2023-11-18.
//

#ifndef INC_3DPERLINMAP_TERRAIN_TOOL_H
#define INC_3DPERLINMAP_TERRAIN_TOOL_H

#include <iostream>
#include <vector>

namespace terrain {
    void
    generate_terrain_vertices(int map_width, int map_height, int patch_numbers, std::vector<float> &vertices);
}

#endif //INC_3DPERLINMAP_TERRAIN_TOOL_H
