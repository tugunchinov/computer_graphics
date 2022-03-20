cmake_minimum_required(VERSION 3.16)
project(Triangle)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
)

add_executable(triangle triangle.cpp)
add_library(shader shader/shader.cpp)

target_include_directories(shader PRIVATE shader)
target_include_directories(triangle PRIVATE shader)

target_link_libraries(triangle OpenGL::GL glfw glm shader)