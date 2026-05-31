cmake_minimum_required(VERSION 3.10)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(i2cat)

add_subdirectory(include/glfw-3.4)
add_subdirectory(include/glm)

add_executable(i2cat_p1 src/main_part1.cpp src/glad.c)
target_link_libraries(i2cat_p1 glfw glm)
target_include_directories(i2cat_p1 PRIVATE include)
target_compile_definitions(i2cat_p1 PRIVATE GL_SILENCE_DEPRECATION)

find_package(OpenGL REQUIRED)
target_link_libraries(i2cat_p1 OpenGL::GL)

add_executable(i2cat_p2 src/main_part2.cpp src/glad.c)
target_link_libraries(i2cat_p2 glfw glm)
target_include_directories(i2cat_p2 PRIVATE include)
target_compile_definitions(i2cat_p2 PRIVATE GL_SILENCE_DEPRECATION)

find_package(OpenGL REQUIRED)
target_link_libraries(i2cat_p2 OpenGL::GL)
