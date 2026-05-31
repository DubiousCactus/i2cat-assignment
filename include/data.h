#ifndef DATA_H
#define DATA_H

#include "happly.h"
#include "shader.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
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

typedef struct {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec3 velocity;
} Point;

glm::vec3 random_velocity_2D_normalized() {
  return glm::normalize(glm::vec3((float)rand() / RAND_MAX * 0.2f - 0.1f,
                                  (float)rand() / RAND_MAX * 0.2f - 0.1f, 0));
}

class PointCloud {
public:
  std::vector<Point> points;
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
      points[i].velocity = glm::vec3(0);
    }
    std::cout << "[*] Binding Vertex Array Object for point cloud '"
              << file_path << "'" << std::endl;
    bindVAO();
  }

  PointCloud(size_t num_points, float velocity_magnitude = 0.1f) {
    points.resize(num_points);
    for (int i = 0; i < num_points; i++) {
      points[i].position =
          glm::vec3((float)rand() / RAND_MAX * 2.0f - 1.0f,
                    (float)rand() / RAND_MAX * 2.0f - 1.0f, 0.0);
      points[i].color =
          glm::vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX,
                    (float)rand() / RAND_MAX);
      points[i].velocity = random_velocity_2D_normalized() * velocity_magnitude;
    }
    bindVAO();
  }

  ~PointCloud() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

  void draw(ShaderProgram shader_program) {
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, points.size());
    glBindVertexArray(0);
  }

  void updateGPUBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Point) * points.size(),
                    points.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
                 GL_DYNAMIC_DRAW);
    // Point positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void *)0);
    // Point cloud
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point),
                          (void *)sizeof(glm::vec3));
    glBindVertexArray(0);
  }
};

class ParticleSystem {
public:
  PointCloud *pcd;

  ParticleSystem(PointCloud *pcd, int n_cells_sqrt, float velocity_magnitude,
                 float padding = 0.01f) {
    this->pcd = pcd;
    this->padding = padding;
    this->indices.resize(pcd->points.size());
    this->cell_assignment.resize(pcd->points.size());
    this->cell_start.resize(n_cells_sqrt * n_cells_sqrt);
    this->cell_end.resize(n_cells_sqrt * n_cells_sqrt);
    this->velocity_magnitude = velocity_magnitude;
    this->n_cells_sqrt = n_cells_sqrt;

    computeCellAssignments();
  }

  void update(float delta_time) {
    /* The particle system is 2D and defines all particles
     * in the normalized space: [-1, 1] for x and y. We
     * bounce particles from the walls of this space. To
     * handle collisions between particles effectively, I
     * chose to subdivide the space in NxN grid cells as
     * to reduce the number of collision checks. Then for
     * every particle, we do the collision check for its
     * neighbours in the cell and the 8 surrounding cells.
     * When a collision occurs, we simply swap the
     * velocity vectors of the two particles.
     */
    for (size_t i = 0; i < pcd->points.size(); i++) {
      // Update the position via velocity integration over time:
      auto &this_pt = pcd->points[i];
      this_pt.position += delta_time * this_pt.velocity;

      // Check collisions within the current cell + 8 surrounding cells:
      //  ___________
      // | 0 | 1 | 2 | 
      // | 3 | 4 | 5 |
      // | 6 | 7 | 8 |
      //  -----------
      //  Cell 4 is the current cell
      size_t cell = cell_assignment[i];
      size_t cell_x = cell % n_cells_sqrt;
      size_t cell_y = floor(cell / n_cells_sqrt);
      for (size_t k = 0; k < 9; k++) {
        // Compute the cell index of the neighbour cell:
        if (cell_x == 0 && (k == 0 || k == 3 || k == 6))
          continue; // Skip left edge cases
        if (cell_x == n_cells_sqrt - 1 && (k == 2 || k == 5 || k == 8))
          continue; // Skip right edge cases
        if (cell_y == 0 && (k == 0 || k == 1 || k == 2))
          continue; // Skip top edge cases
        if (cell_y == n_cells_sqrt - 1 && (k == 6 || k == 7 || k == 8))
          continue; // Skip bottom edge cases
        size_t neighbour_cell_x = cell_x + (k % 3) - 1;
        size_t neighbour_cell_y = cell_y + floor(k / 3) - 1;
        size_t neighbour_cell = neighbour_cell_y * n_cells_sqrt + neighbour_cell_x;
        // Go through all points in this cell, and check for collisions:
        for (size_t j = cell_start[neighbour_cell]; j <= cell_end[neighbour_cell]; j++) {
          if (i == indices[j])
            continue; // Don't check collision with itself
          auto &pt = pcd->points[indices[j]];
          if (collision(this_pt, pt)) {
            // Swap velocities and add some noise to prevent re-swapping
            glm::vec3 temp = this_pt.velocity;
            this_pt.velocity =
                pt.velocity + random_velocity_2D_normalized() * 0.5f;
            pt.velocity = temp + random_velocity_2D_normalized() * 0.5f;
            // Normalize the output velocity and multiply by a constant
            // magnitude for all particles. Otherwise this will add up and
            // explode!
            pt.velocity = glm::normalize(pt.velocity) * velocity_magnitude;
            this_pt.velocity =
                glm::normalize(this_pt.velocity) * velocity_magnitude;
          }
        }
      }

      // Constrain the particle to the unit box:
      if (this_pt.position.y + padding > 1.0f) {
        this_pt.position.y = 1.0f - padding;
        this_pt.velocity.y = -std::abs(this_pt.velocity.y);
      } else if (this_pt.position.y - padding < -1.0f) {
        this_pt.position.y = -1.0f + padding;
        this_pt.velocity.y = std::abs(this_pt.velocity.y);
      }
      if (this_pt.position.x + padding > 1.0f) {
        this_pt.position.x = 1.0f - padding;
        this_pt.velocity.x = -std::abs(this_pt.velocity.x);
      } else if (this_pt.position.x - padding < -1.0f) {
        this_pt.position.x = -1.0f + padding;
        this_pt.velocity.x = std::abs(this_pt.velocity.x);
      }
    }
    computeCellAssignments();
    pcd->updateGPUBuffer();
  }

private:
  size_t n_cells_sqrt;
  float padding;
  float velocity_magnitude;
  std::vector<size_t> indices;
  std::vector<size_t> cell_assignment;
  std::vector<size_t> cell_start;
  std::vector<size_t> cell_end;

  bool collision(Point p1, Point p2) {
    float distance = glm::length(p1.position - p2.position);
    return distance < padding;
  }

  void computeCellAssignments() {
    // position x/y range from [-1, 1], so we can compute the grid cell index
    // for each particle by first bringing the position to the [0, 1] range and
    // then using the formula: id = y * n_cells_y * n_cells_x + x.
    for (size_t i = 0; i < pcd->points.size(); i++) {
      size_t cell_x = std::min(
          static_cast<size_t>(floor(
              ((pcd->points[i].position.x + 1.0f) / 2.0f) * n_cells_sqrt)),
          n_cells_sqrt - 1);
      size_t cell_y = std::min(
          static_cast<size_t>(floor(
              ((pcd->points[i].position.y + 1.0f) / 2.0f) * n_cells_sqrt)),
          n_cells_sqrt - 1);
      cell_assignment[i] = cell_y * n_cells_sqrt + cell_x;
      indices[i] = i;
    }
    // Now we have a list of cell assignments for each point, and a list of
    // point indices. We want to sort the indices by the cell assignments such
    // that cell points are contiguous in memory. This makes it cache friendly
    // so good for performance.
    std::sort(indices.begin(), indices.end(), [this](size_t a, size_t b) {
      return cell_assignment[a] < cell_assignment[b];
    });
    // And finally we store the start and end index of each cell in the
    // cell_start and cell_end arrays. This index is for the indices array, ie
    // the offsets for each cell. We now have a simple and performant solution
    // to access all the particles in a given cell.
    size_t no_particles = indices.size();
    std::fill(cell_start.begin(), cell_start.end(), no_particles);
    std::fill(cell_end.begin(), cell_end.end(), no_particles);
    for (size_t i = 0; i < indices.size(); i++) {
      size_t cell = cell_assignment[indices[i]];
      if (cell_start[cell] == no_particles) {
        cell_start[cell] = i;
      }
      cell_end[cell] = i + 1;
    }
  }
};

#endif /* DATA_H */
