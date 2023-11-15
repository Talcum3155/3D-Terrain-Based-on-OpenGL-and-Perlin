//
// Created by Tarowy on 2023-11-13.
//

#ifndef INC_3DPERLIN_mAP_SHADER_H
#define INC_3DPERLIN_mAP_SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <istream>
#include <sstream>
#include <vector>
#include <algorithm>

namespace utilities {

    class shader {
    public:
        unsigned int id = 0;
        bool compiled_flag = false;

        inline shader(std::string &&absolute_path, std::string &&vert_path, std::string &&frag_path);

        ~shader() = default;

        void build_shader();

        inline void use();

        inline void set_bool(const std::string &name, bool value) const;

        inline void set_int(const std::string &name, int value) const;

        inline void set_float(const std::string &name, float value) const;

        inline void set_vec2(const std::string &name, glm::vec2 &value) const;

        inline void set_vec2(const std::string &name, float x, float y) const;

        inline void set_vec3(const std::string &name, const glm::vec3 &value) const;

        inline void set_vec3(const std::string &name, float x, float y, float z) const;

        inline void set_vec4(const std::string &name, const glm::vec4 &value) const;

        inline void set_vec4(const std::string &name, float x, float y, float z, float w) const;

        inline void set_mat2(const std::string &name, const glm::mat2 &mat) const;

        inline void set_mat3(const std::string &name, const glm::mat3 &mat) const;

        inline void set_mat4(const std::string &name, const glm::mat4 &mat) const;

    protected:
        // store all paths
        std::vector<std::string> shader_paths;

        /**
         * Override by an inherited class to add shader types, like tese and tesc.
         * @param shader_ids
         * @param shader_codes
         */
        inline virtual void
        create_compile_shader_delegate(std::vector<unsigned int> &shader_ids,
                                       std::vector<std::string> &shader_codes) const;

        static unsigned int
        create_compile_shader(std::string &shader_code, char32_t gl_shader_type,
                              std::string &&check_shader_type);

    private:
        static void check_compiler_errors(unsigned int shader_id, std::string &&shader_type);

        static std::string load_shader_code_from_file(std::string &shader_path);
    };

    inline
    shader::shader(std::string &&absolute_path, std::string &&vert_path, std::string &&frag_path) {
        shader_paths.emplace_back(absolute_path + vert_path);
        shader_paths.emplace_back(absolute_path + frag_path);
    }

    /**
     * Override by an inherited class to add shader types, like tese and tesc.
     * @param shader_ids
     * @param shader_codes
     */
    inline void
    shader::create_compile_shader_delegate(std::vector<unsigned int> &shader_ids,
                                           std::vector<std::string> &shader_codes) const {
    }

    /**
     * Use the shader, If the compilation is not completed, it will proceed with the compilation.
     */
    inline void
    shader::use() {
        if (!compiled_flag)
            build_shader();
        glUseProgram(id);
    }

#pragma region control_shaders

    inline void
    shader::set_bool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<int>(value));
    }

    inline void
    shader::set_int(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }

    inline void
    shader::set_float(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }

    inline void
    shader::set_vec2(const std::string &name, glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }

    inline void
    shader::set_vec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
    }

    inline void
    shader::set_vec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }

    inline void
    shader::set_vec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
    }

    inline void
    shader::set_vec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }

    inline void
    shader::set_vec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
    }

    inline void
    shader::set_mat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    inline void
    shader::set_mat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    inline void
    shader::set_mat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

#pragma endregion control_shaders

}

#endif //INC_3DPERLIN_mAP_SHADER_H
