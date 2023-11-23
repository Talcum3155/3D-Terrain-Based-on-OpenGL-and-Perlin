//
// Created by Tarowy on 2023-11-23.
//

#ifndef INC_3DPERLINMAP_MAP_CHUNK_H
#define INC_3DPERLINMAP_MAP_CHUNK_H

#include <vector>
#include <iostream>

namespace terrain {

    struct pair_hash {
        template<class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &pair) const {
            auto hash1 = std::hash<T1>{}(pair.first);
            auto hash2 = std::hash<T2>{}(pair.second);
            return hash1 ^ hash2;
        }
    };

    struct map_chunk {
    public:
        int grid_x;
        int grid_y;
        std::vector<float> height_data;
        unsigned int height_map_id = 0;

        map_chunk(int grid_x, int grid_y)
                : grid_x(grid_x), grid_y(grid_y) {
        }

        map_chunk(int grid_x, int grid_y, std::vector<float> &&height_data)
                : grid_x(grid_x), grid_y(grid_y), height_data(height_data) {
        }

        map_chunk(int grid_x, int grid_y, std::vector<float> &&height_data, unsigned int height_map_id)
                : grid_x(grid_x), grid_y(grid_y), height_data(height_data), height_map_id(height_map_id) {
        }

        ~map_chunk() = default;

        friend std::ostream &operator<<(std::ostream &os, const map_chunk &chunk) {
            os << '(' << chunk.grid_x << ", " << chunk.grid_y << ')';
            return os;
        }
    };

}

#endif //INC_3DPERLINMAP_MAP_CHUNK_H
