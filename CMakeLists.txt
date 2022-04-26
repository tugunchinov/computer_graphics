cmake_minimum_required(VERSION 3.16)
project(Game)

find_package(OpenGL REQUIRED)

if (WIN32)
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

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

add_executable(game main.cpp)
add_library(shader shader/shader.cpp)
add_library(controls controls/controls.cpp)
add_library(texture texture/texture.cpp)
add_library(objloader objloader/objloader.cpp)

target_link_libraries(controls PUBLIC glfw glm)
target_link_libraries(texture PUBLIC glfw glm)
target_link_libraries(objloader PUBLIC glm)

configure_file(shader/enemy_vertex_shader shader/enemy_vertex_shader)
configure_file(shader/enemy_fragment_shader shader/enemy_fragment_shader)
configure_file(shader/fireball_vertex_shader shader/fireball_vertex_shader)
configure_file(shader/fireball_fragment_shader shader/fireball_fragment_shader)

configure_file(sphere.obj sphere.obj)

include_directories(external/glew-2.2.0/include)

target_include_directories(shader PRIVATE shader)
target_include_directories(controls PRIVATE controls)
target_include_directories(texture PRIVATE texture)
target_include_directories(objloader PRIVATE objloader)
target_include_directories(game PRIVATE shader controls sphere objloader texture)

target_link_libraries(game PUBLIC OpenGL::GL glfw glm shader texture objloader controls glew_s)

set_property(TARGET game PROPERTY CXX_STANDARD 20)

add_custom_command(TARGET game PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/RAY.BMP ${PROJECT_BINARY_DIR}/RAY.BMP
        )