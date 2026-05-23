cmake_minimum_required(VERSION 3.10)
project(i2cat)

add_subdirectory(include/glfw-3.4)
add_subdirectory(include/glm)

add_executable(i2cat src/main.cpp src/glad.c)
target_link_libraries(i2cat glfw glm)
target_include_directories(i2cat PRIVATE include)
target_compile_definitions(i2cat PRIVATE GL_SILENCE_DEPRECATION)

find_package(OpenGL REQUIRED)
target_link_libraries(i2cat OpenGL::GL)
