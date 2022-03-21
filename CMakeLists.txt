cmake_minimum_required(VERSION 3.16)
project(Triangle)

find_package(OpenGL REQUIRED)

if (WIN32)
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/glew-2.2.0/build/cmake)

add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
)

add_executable(triangle triangle.cpp)
add_library(shader shader/shader.cpp)

configure_file(shader/vertex_shader_1 shader/vertex_shader_1)
configure_file(shader/vertex_shader_2 shader/vertex_shader_2)
configure_file(shader/fragment_shader_1 shader/fragment_shader_1)
configure_file(shader/fragment_shader_2 shader/fragment_shader_2)

include_directories(external/glew-2.2.0/include)

target_include_directories(shader PRIVATE shader)
target_include_directories(triangle PRIVATE shader)

target_link_libraries(triangle OpenGL::GL glfw glm shader GLEW)