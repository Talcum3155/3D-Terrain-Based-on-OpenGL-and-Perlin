//
// Created by Tarowy on 2023-11-15.
//

#ifndef INC_3DPERLINMAP_SHADER_T_H
#define INC_3DPERLINMAP_SHADER_T_H

#include "shader.h"

namespace utilities {

    class shader_t : public shader {
    public:
        inline shader_t(std::string &&absolute_path, std::string &&vert_name,
                        std::string &&frag_name, std::string &&tesc_name, std::string &&tese_name);

        ~shader_t() = default;

    protected:

        inline void
        create_compile_shader_delegate(std::vector<unsigned int> &shader_ids,
                                       std::vector<std::string> &shader_codes) const override;
    };

    inline
    shader_t::shader_t(std::string &&absolute_path,
                       std::string &&vert_name, std::string &&frag_name,
                       std::string &&tesc_name, std::string &&tese_name)
            : shader(std::forward<std::string>(absolute_path),
                     std::forward<std::string>(vert_name),
                     std::forward<std::string>(frag_name)) {
        // Ensure the correct order of shaders
        shader_paths.insert(shader_paths.begin() + 1, absolute_path + tese_name);
        shader_paths.insert(shader_paths.begin() + 1, absolute_path + tesc_name);
    }

    /**
     * Adding shaders specific to the subclass.
     * @param shader_ids
     * @param shader_codes
     */
    inline void
    shader_t::create_compile_shader_delegate(std::vector<unsigned int> &shader_ids,
                                             std::vector<std::string> &shader_codes) const {
        shader_ids.push_back(
                create_compile_shader(shader_codes[shader_ids.size()], GL_TESS_CONTROL_SHADER, "TESS_CONTROL"));
        shader_ids.push_back(
                create_compile_shader(shader_codes[shader_ids.size()], GL_TESS_EVALUATION_SHADER, "TESS_EVALUATION"));
    }
}


#endif //INC_3DPERLINMAP_SHADER_T_H
