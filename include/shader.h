#ifndef SHADER_H
#define SHADER_H

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram {
public:
  ShaderProgram(std::string vertex_shader_path,
                std::string fragment_shader_path) {
    std::cout << "[*] Loading vertex shader program from " << vertex_shader_path
              << "..." << std::endl;
    std::cout << "[*] Loading fragment shader program from "
              << fragment_shader_path << "..." << std::endl;
    std::string v_code;
    std::string f_code;
    std::ifstream v_shader_file;
    std::ifstream f_shader_file;
    // ensure ifstream objects can throw exceptions:
    v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
      // open files
      v_shader_file.open(vertex_shader_path);
      f_shader_file.open(fragment_shader_path);
      std::stringstream v_shader_stream, f_shader_stream;
      // read file's buffer contents into streams
      v_shader_stream << v_shader_file.rdbuf();
      f_shader_stream << f_shader_file.rdbuf();
      // close file handlers
      v_shader_file.close();
      f_shader_file.close();
      // convert stream into string
      v_code = v_shader_stream.str();
      f_code = f_shader_stream.str();
    } catch (std::ifstream::failure e) {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
      throw std::runtime_error("ShaderProgram: can't read file!");
    }
    const char *v_shader_code = v_code.c_str();
    const char *f_shader_code = f_code.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_shader_code, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vertex, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                << infoLog << std::endl;
      throw std::runtime_error("ShaderProgram: compilation failed!");
    };

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_shader_code, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fragment, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                << infoLog << std::endl;
      throw std::runtime_error("ShaderProgram: compilation failed!");
    };

    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    // print linking errors if any
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(id, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                << infoLog << std::endl;
      throw std::runtime_error("ShaderProgram: linking failed!");
    }

    // delete the shaders as they're linked into our program now and no longer
    // necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
  }

  void use() { glUseProgram(id); }

  void setMat4(std::string name, glm::mat4 mat, bool transpose) {
    GLuint loc = glGetUniformLocation(id, name.c_str());
    if (loc == -1) {
      throw std::runtime_error("failed to find uniform: " + name);
    }
    glUniformMatrix4fv(loc, 1, transpose, glm::value_ptr(mat));
  }

private:
  GLuint id;
};

#endif /* SHADER_H */
