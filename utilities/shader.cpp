//
// Created by Tarowy on 2023-11-13.
//

#include "shader.h"

namespace utilities {

    /**
     * Load shader code, Create and Compile shader
     * and Delete unuseful shaders
     */
    void
    shader::build_shader() {
        std::vector<std::string> shader_codes;

        // load all shader codes
        std::for_each(shader_paths.begin(), shader_paths.end(),
                      [&](auto &path) { shader_codes.push_back(load_shader_code_from_file(path)); }
        );

        std::vector<unsigned int> shader_ids;

        // For each additional shader, the capacity of shader_id will increase by 1.
        shader_ids.push_back(
                create_compile_shader(shader_codes[shader_ids.size()], GL_VERTEX_SHADER, "VERTX"));

        // override by inherited class to add shader type, like tese,tesc
        create_compile_shader_delegate(shader_ids, shader_codes);

        shader_ids.push_back(
                create_compile_shader(shader_codes[shader_ids.size()], GL_FRAGMENT_SHADER, "FRAGMENT"));


        // shader Program
        id = glCreateProgram();

        // attach every shader to
        std::for_each(shader_ids.begin(), shader_ids.end(),
                      [&](const auto &shader_id) { glAttachShader(id, shader_id); }
        );

        glLinkProgram(id);

        check_compiler_errors(id, "PROGRAM");

        // delete the shaders as they're linked into our program now and no longer necessary
        std::for_each(shader_ids.begin(), shader_ids.end(),
                      [&](const auto &shader_id) { glDeleteShader(shader_id); }
        );

        compiled_flag = true;
    }

    /**
     * create shader and compile it
     * @param shader_code
     * @param gl_shader_type
     * @param check_shader_type
     * @return shader id
     */
    unsigned int
    shader::create_compile_shader(std::string &shader_code,
                                  char32_t gl_shader_type,
                                  std::string &&check_shader_type) {
        // create shader
        unsigned int shader_id = glCreateShader(gl_shader_type);
        const char *shader = shader_code.c_str();
        glShaderSource(shader_id, 1, &shader, nullptr);
        // compile shader
        glCompileShader(shader_id);
        // check compile error
        check_compiler_errors(shader_id, std::forward<std::string>(check_shader_type));
        return shader_id;
    }

    /**
     * load shader code from path
     * @param shader_path
     * @param shader_code
     */
    std::string
    shader::load_shader_code_from_file(std::string &shader_path) {
        std::ifstream shader_file;
        std::stringstream shader_stream;

        // ensure ifstream objects can throw exceptions
        shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            shader_file.open(shader_path);
            std::cout << "Reading shader codes from the path: " << shader_path << std::endl;
            shader_stream << shader_file.rdbuf();
            shader_file.close();
        } catch (std::ifstream::failure &error) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << error.what() << std::endl;
        }

        return shader_stream.str();
    }

    /**
     * check compiler error, program status and link status
     * @param shader_id shader's id
     * @param shader_type which shader to compile
     */
    void
    shader::check_compiler_errors(unsigned int shader_id, std::string &&shader_type) {
        int success_flag = false;
        char info_log[256];

        // Check for errors after compile.
        if (shader_type != "PROGRAM") {
            std::cout << "Checking errors for " << shader_type << "..." << std::endl;
            glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success_flag);

            if (!success_flag) {
                // get error info
                glGetShaderInfoLog(shader_id, sizeof(info_log), nullptr, info_log);
                throw std::runtime_error(
                        std::string("ERROR::SHADER_COMPILATION_ERROR of type: ") + shader_type + "\n" + info_log);
            }
        }
            // Check for errors during shader compilation.
        else {
            std::cout << "Checking errors for PROGRAM..." << std::endl;
            glGetProgramiv(shader_id, GL_LINK_STATUS, &success_flag);

            if (!success_flag) {
                // get error info
                glGetProgramInfoLog(shader_id, sizeof(info_log), nullptr, info_log);
                throw std::runtime_error(
                        std::string("ERROR::PROGRAM_LINKING_ERROR of type: \n") + shader_type + "\n" + info_log);
            }
        }
    }
}
