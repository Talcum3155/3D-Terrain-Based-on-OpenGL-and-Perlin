//
// Created by Tarowy on 2023-11-20.
//

#ifndef INC_3DPERLINMAP_SHADER_G_T_H
#define INC_3DPERLINMAP_SHADER_G_T_H

#include "shader_t.h"

namespace utilities {

    class shader_g_t : public shader_t {
    public:
        inline shader_g_t(std::string &&absolute_path, std::string &&vert_name,
                          std::string &&frag_name, std::string &&tesc_name, std::string &&tese_name,
                          std::string &&geom_name);

        ~shader_g_t() = default;

    protected:

        inline void
        create_compile_shader_delegate(std::vector<unsigned int> &shader_ids,
                                       std::vector<std::string> &shader_codes) const override;
    };

    shader_g_t::shader_g_t(std::string &&absolute_path, std::string &&vert_name, std::string &&frag_name,
                           std::string &&tesc_name, std::string &&tese_name, std::string &&geom_name)
            : shader_t(std::forward<std::string>(absolute_path), std::forward<std::string>(vert_name),
                       std::forward<std::string>(frag_name), std::forward<std::string>(tesc_name),
                       std::forward<std::string>(tese_name)) {
        shader_paths.insert(shader_paths.begin() + 3, absolute_path + geom_name);
    }

    void shader_g_t::create_compile_shader_delegate(std::vector<unsigned int> &shader_ids,
                                                    std::vector<std::string> &shader_codes) const {
        shader_t::create_compile_shader_delegate(shader_ids, shader_codes);
        shader_ids.push_back(
                create_compile_shader(shader_codes[shader_ids.size()], GL_GEOMETRY_SHADER, "GEOMETRY"));
    }

}

#endif //INC_3DPERLINMAP_SHADER_G_T_H
