cmake_minimum_required(VERSION 3.16)
project(Triangle)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

add_executable(triangle triangle.cpp)
add_library(shader shader/shader.cpp)

configure_file(shader/vertex_shader shader/vertex_shader)
configure_file(shader/fragment_shader shader/fragment_shader)

target_include_directories(shader PRIVATE shader)
target_include_directories(triangle PRIVATE shader)

target_link_libraries(triangle OpenGL::GL glfw glm shader GLEW)