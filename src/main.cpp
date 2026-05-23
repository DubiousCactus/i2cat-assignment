// clang-format off
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include "data.h"
#include "shader.h"
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/mat4x4.hpp>               // glm::mat4
#include <glm/vec3.hpp>                 // glm::vec3
#include <glm/vec4.hpp>                 // glm::vec4
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

glm::mat4 makeProjectionViewCamera(glm::vec3 eye, glm::vec3 target) {
  glm::vec3 dir = glm::normalize(eye - target);
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 right = glm::normalize(glm::cross(up, dir));
  glm::vec3 cam_up = glm::cross(dir, right);
  glm::mat4 view = glm::lookAt(eye, target, cam_up);
  glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                    (float)640 / (float)480, 0.1f, 100.0f);
  return proj * view;
}

int main(void) {
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glEnable(GL_DEPTH_TEST);

  glm::mat4 camera = makeProjectionViewCamera(glm::vec3(0.0f, 10.0f, 30.0f),
                                              glm::vec3(0.0f, 0.0f, 0.0f));

  ShaderProgram shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
  PointCloud pcd("./Loot.ply");

  // This transformation rotates it smoothly at every frame:
  glm::mat4 rotation_M = glm::mat4(1.0f);
  // This transformation translates it smoothly at every frame:
  glm::mat4 translation_M = glm::mat4(1.0f);
  // This transformation brings it to the center of the image:
  glm::mat4 canonical_M =
      glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -6.0f, 0.0f)),
                 glm::vec3(0.01f));

  float last_frame = 0.0f;
  const float rot_speed = 2.0f;
  const float t_strength = 1000.0f;
  while (!glfwWindowShouldClose(window)) {
    float current_frame = glfwGetTime();
    float delta_time = current_frame - last_frame;
    last_frame = current_frame;
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    rotation_M = glm::rotate(rotation_M, rot_speed * 0.1f * 3.14f * delta_time,
                             glm::vec3(0, 1.0f, 0.0f));
    translation_M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, sin(current_frame) * t_strength));
    pcd.draw(shader, camera, canonical_M * translation_M * rotation_M);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
