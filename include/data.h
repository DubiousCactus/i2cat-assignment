#ifndef DATA_H
#define DATA_H

#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/mat4x4.hpp>               // glm::mat4
#include <glm/vec3.hpp>                 // glm::vec3
#include <glm/vec4.hpp>                 // glm::vec4
#include <string>
#include <sys/_types/_u_char.h>
#include <sys/types.h>
#include <vector>
#include "happly.h"
#include "shader.h"

typedef struct {
  glm::vec3 position;
  glm::vec3 color;
} Point;

class PointCloud {
  std::vector<Point> points;

public:
  PointCloud(std::string file_path) {
    // Load points from file
    std::cout << "[*] Parsing point cloud file '" << file_path << "'"
              << std::endl;
    happly::PLYData plyFile(file_path);

    if (!plyFile.hasElement("vertex")) {
      throw std::runtime_error(
          "Error: PLY file does not contain 'vertex' element.");
    }

    auto &vertex_el = plyFile.getElement("vertex");

    for (auto name : vertex_el.getPropertyNames()) {
      std::cout << "PLY element 'vertex' has property: " << name << std::endl;
    }
    std::vector<double> x = vertex_el.getProperty<double>("x");
    std::vector<double> y = vertex_el.getProperty<double>("y");
    std::vector<double> z = vertex_el.getProperty<double>("z");

    std::vector<u_char> r = vertex_el.getProperty<u_char>("red");
    std::vector<u_char> g = vertex_el.getProperty<u_char>("green");
    std::vector<u_char> b = vertex_el.getProperty<u_char>("blue");

    points.resize(x.size());
    for (size_t i = 0; i < points.size(); i++) {
      points[i].position = glm::vec3(x[i], y[i], z[i]);
      points[i].color = glm::vec3((float)r[i] / 255.0f, (float)g[i] / 255.0f,
                                  (float)b[i] / 255.0f);
    }
    std::cout << "[*] Binding Vertex Array Object for point cloud '"
              << file_path << "'" << std::endl;
    bindVAO();
  }

  ~PointCloud() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

  void draw(ShaderProgram shader_program, glm::mat4 camera_proj_view,
            glm::mat4 model_mat) {
    try {
      shader_program.setMat4("u_proj_view", camera_proj_view, false);
    } catch (int err) {
      std::cout << "Can't set u_proj_view!" << std::endl;
    }
    try {
      shader_program.setMat4("u_model", model_mat, false);
    } catch (int err) {
      std::cout << "Can't set u_model!" << std::endl;
    }

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, points.size());
    glBindVertexArray(0);
  }

private:
  GLuint vao, vbo;

  void bindVAO() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * points.size(), points.data(),
                 GL_STATIC_DRAW);
    // Point positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void *)0);
    // Point clour
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point),
                          (void *)sizeof(glm::vec3));
    glBindVertexArray(0);
  }
};

#endif /* DATA_H */
