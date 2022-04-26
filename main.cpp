#include <chrono>
#include <fstream>
#include <iostream>
#include <random>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <controls.hpp>
#include <objloader.hpp>
#include <shader.hpp>
#include <texture.hpp>

using namespace std::chrono_literals;

static constexpr float kFireBallSpeed = 0.1f;
static constexpr float kCollisionRadius = 0.67f;
static constexpr float kFarRadius = 42.0f;

extern glm::vec3 position;
extern float horizontal_angle;
extern float vertical_angle;

glm::quat GetRandomQuaternion() {
  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> dist{0.0f, 1.0f};

  float u = dist(rng);
  float v = dist(rng);
  float w = dist(rng);

  return {std::sqrt(1.0f - u) * std::sin(2.0f * M_PI * v),
          std::sqrt(1.0f - u) * std::cos(2.0f * M_PI * v),
          std::sqrt(u) * std::sin(2.0f * M_PI * w),
          std::sqrt(u) * std::cos(2.0f * M_PI * v)};
}

glm::vec3 GetRandomPosition() {
  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> dist{-7.0f, 7.0f};

  float u = dist(rng);
  float v = dist(rng);
  float w = dist(rng);

  return {u, v, w};
}

int main() {

  /*** INITIALIZATION ***/

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(1024, 768, "GAME", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Failed to open GLFW window. If you have an Intel GPU, they "
                 "are not 3.3 compatible. Try the 2.1 version of the tutorials."
              << std::endl;
    glfwTerminate();
    return 2;
  }
  glfwMakeContextCurrent(window);

  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    glfwTerminate();
    return 3;
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  GLuint vertex_array_id;
  glGenVertexArrays(1, &vertex_array_id);
  glBindVertexArray(vertex_array_id);

  /*** END INITIALIZATION ***/

  /*** ENEMY OBJECT ***/

  // pyramid
  static const GLfloat g_vertex_buffer_data[] = {
      // 1
      1.0f, 0.0f, 0.0f, // vertex 1
      0.0f, 1.0f, 0.0f, // vertex 2
      0.0f, 0.0f, 1.0f, // vertex 3

      // 2
      -1.0f, 0.0f, 0.0f, // vertex 1
      0.0f, 1.0f, 0.0f,  // vertex 2
      0.0f, 0.0f, 1.0f,  // vertex 3

      // 3
      -1.0f, 0.0f, 0.0f, // vertex 1
      0.0f, -1.0f, 0.0f, // vertex 2
      0.0f, 0.0f, 1.0f,  // vertex 3

      // 4
      1.0f, 0.0f, 0.0f,  // vertex 1
      0.0f, -1.0f, 0.0f, // vertex 2
      0.0f, 0.0f, 1.0f,  // vertex 3

      // 5
      1.0f, 0.0f, 0.0f,  // vertex 1
      0.0f, -1.0f, 0.0f, // vertex 2
      0.0f, 1.0f, 0.0f,  // vertex 3

      // 6
      -1.0f, 0.0f, 0.0f, // vertex 1
      0.0f, -1.0f, 0.0f, // vertex 2
      0.0f, 1.0f, 0.0f,  // vertex 3
  };

  GLuint vertex_buffer;
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
               g_vertex_buffer_data, GL_STATIC_DRAW);

  /*** END ENEMY OBJECT ***/

  /*** ENEMY COLOR ***/

  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> dist{0.0f, 1.0f};

  static GLfloat g_color_buffer_data[6 * 9]{};
  for (float &i : g_color_buffer_data) {
    i = dist(rng);
  }

  GLuint color_buffer;
  glGenBuffers(1, &color_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data),
               g_color_buffer_data, GL_STATIC_DRAW);

  /*** END ENEMY COLOR ***/

  /*** ENEMY POSITION ***/

  std::vector<glm::vec3> g_enemies_position_data;

  GLuint enemies_position_buffer;
  glGenBuffers(1, &enemies_position_buffer);

  /*** END ENEMY POSITION ***/

  /*** ENEMY ROTATION ***/

  std::vector<glm::quat> g_enemies_rotation_data;

  GLuint enemies_rotation_buffer;
  glGenBuffers(1, &enemies_rotation_buffer);

  /*** END ENEMY ROTATION ***/

  std::vector<glm::vec3> fireball_vertices;
  std::vector<glm::vec2> fireball_uvs;
  std::vector<glm::vec3> fireball_normals;
  loadOBJ("sphere.obj", fireball_vertices, fireball_uvs, fireball_normals);

  for (auto &vert : fireball_vertices) {
    vert /= 20.0f;
  }

  /*** FIREBALL VERTICES ***/

  std::vector<glm::vec3> g_fireball_vertex_data;

  GLuint fireball_vertex_buffer;
  glGenBuffers(1, &fireball_vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, fireball_vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, fireball_vertices.size() * sizeof(glm::vec3),
               fireball_vertices.data(), GL_STATIC_DRAW);

  /*** END FIREBALL VERTICES ***/

  /*** FIREBALL POSITION ***/

  std::vector<glm::vec3> g_fireball_position_data;

  GLuint fireball_position_buffer;
  glGenBuffers(1, &fireball_position_buffer);

  /*** END FIREBALL POSITION ***/

  /*** FIREBALL DIRECTION ***/

  std::vector<glm::vec3> g_fireball_direction_data;

  /*** END FIREBALL DIRECTION ***/

  /*** FIREBALL UV ***/

  GLuint fireball_uv_buffer;
  glGenBuffers(1, &fireball_uv_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, fireball_uv_buffer);
  glBufferData(GL_ARRAY_BUFFER, fireball_uvs.size() * sizeof(glm::vec2),
               fireball_uvs.data(), GL_STATIC_DRAW);

  /*** END FIREBALL UV ***/

  /*** SHADERS INITIALIZATION ***/
  GLuint enemy_shader = LoadShaders("./shader/enemy_vertex_shader",
                                    "./shader/enemy_fragment_shader");
  GLint matrix_id_enemy = glGetUniformLocation(enemy_shader, "MVP");

  GLuint fireball_shader = LoadShaders("./shader/fireball_vertex_shader",
                                       "./shader/fireball_fragment_shader");
  GLint matrix_id_fireball = glGetUniformLocation(fireball_shader, "MVP");

  GLuint fireball_texture_id =
      glGetUniformLocation(fireball_shader, "fireball_texture");

  GLuint texture = loadBMP_custom("RAY.BMP");

  /*** END SHADERS INITIALIZATION ***/

  g_enemies_position_data.emplace_back(GetRandomPosition());
  g_enemies_rotation_data.push_back(GetRandomQuaternion());

  auto start = glfwGetTime();
  bool mouse_pressed = false;
  bool mouse_released = true;
  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*** EVENTS HANDLING ***/

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      std::ofstream save_file("dump.save", std::ios::binary);

      size_t size = g_enemies_position_data.size();
      save_file.write(reinterpret_cast<char *>(&size), sizeof(size_t));
      for (auto &elem : g_enemies_position_data) {
        save_file.write(reinterpret_cast<char *>(&elem.x), sizeof(elem.x));
        save_file.write(reinterpret_cast<char *>(&elem.y), sizeof(elem.y));
        save_file.write(reinterpret_cast<char *>(&elem.z), sizeof(elem.z));
      }

      size = g_enemies_rotation_data.size();
      save_file.write(reinterpret_cast<char *>(&size), sizeof(size_t));
      for (auto &elem : g_enemies_rotation_data) {
        save_file.write(reinterpret_cast<char *>(&elem.x), sizeof(elem.x));
        save_file.write(reinterpret_cast<char *>(&elem.y), sizeof(elem.y));
        save_file.write(reinterpret_cast<char *>(&elem.z), sizeof(elem.z));
        save_file.write(reinterpret_cast<char *>(&elem.w), sizeof(elem.w));
      }

      size = g_fireball_position_data.size();
      save_file.write(reinterpret_cast<char *>(&size), sizeof(size_t));
      for (auto &elem : g_fireball_position_data) {
        save_file.write(reinterpret_cast<char *>(&elem.x), sizeof(elem.x));
        save_file.write(reinterpret_cast<char *>(&elem.y), sizeof(elem.y));
        save_file.write(reinterpret_cast<char *>(&elem.z), sizeof(elem.z));
      }

      size = g_fireball_direction_data.size();
      save_file.write(reinterpret_cast<char *>(&size), sizeof(size_t));
      for (auto &elem : g_fireball_direction_data) {
        save_file.write(reinterpret_cast<char *>(&elem.x), sizeof(elem.x));
        save_file.write(reinterpret_cast<char *>(&elem.y), sizeof(elem.y));
        save_file.write(reinterpret_cast<char *>(&elem.z), sizeof(elem.z));
      }

      save_file.write(reinterpret_cast<char *>(&position.x),
                      sizeof(position.x));
      save_file.write(reinterpret_cast<char *>(&position.y),
                      sizeof(position.y));
      save_file.write(reinterpret_cast<char *>(&position.z),
                      sizeof(position.z));

      save_file.write(reinterpret_cast<char *>(&horizontal_angle),
                      sizeof(float));
      save_file.write(reinterpret_cast<char *>(&vertical_angle), sizeof(float));

      save_file.close();
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
      std::ifstream save_file("dump.save", std::ios::binary);

      size_t size = g_enemies_position_data.size();
      save_file.read(reinterpret_cast<char *>(&size), sizeof(size_t));
      g_enemies_position_data.resize(size);
      for (auto &elem : g_enemies_position_data) {
        save_file.read(reinterpret_cast<char *>(&elem.x), sizeof(elem.x));
        save_file.read(reinterpret_cast<char *>(&elem.y), sizeof(elem.y));
        save_file.read(reinterpret_cast<char *>(&elem.z), sizeof(elem.z));
      }

      size = g_enemies_rotation_data.size();
      save_file.read(reinterpret_cast<char *>(&size), sizeof(size_t));
      g_enemies_rotation_data.resize(size);
      for (auto &elem : g_enemies_rotation_data) {
        save_file.read(reinterpret_cast<char *>(&elem.x), sizeof(elem.x));
        save_file.read(reinterpret_cast<char *>(&elem.y), sizeof(elem.y));
        save_file.read(reinterpret_cast<char *>(&elem.z), sizeof(elem.z));
        save_file.read(reinterpret_cast<char *>(&elem.w), sizeof(elem.w));
      }

      size = g_fireball_position_data.size();
      save_file.read(reinterpret_cast<char *>(&size), sizeof(size_t));
      g_fireball_position_data.resize(size);
      for (auto &elem : g_fireball_position_data) {
        save_file.read(reinterpret_cast<char *>(&elem.x), sizeof(elem.x));
        save_file.read(reinterpret_cast<char *>(&elem.y), sizeof(elem.y));
        save_file.read(reinterpret_cast<char *>(&elem.z), sizeof(elem.z));
      }

      size = g_fireball_direction_data.size();
      save_file.read(reinterpret_cast<char *>(&size), sizeof(size_t));
      g_fireball_direction_data.resize(size);
      for (auto &elem : g_fireball_direction_data) {
        save_file.read(reinterpret_cast<char *>(&elem.x), sizeof(elem.x));
        save_file.read(reinterpret_cast<char *>(&elem.y), sizeof(elem.y));
        save_file.read(reinterpret_cast<char *>(&elem.z), sizeof(elem.z));
      }

      save_file.read(reinterpret_cast<char *>(&position.x), sizeof(position.x));
      save_file.read(reinterpret_cast<char *>(&position.y), sizeof(position.y));
      save_file.read(reinterpret_cast<char *>(&position.z), sizeof(position.z));

      save_file.read(reinterpret_cast<char *>(&horizontal_angle),
                     sizeof(float));
      save_file.read(reinterpret_cast<char *>(&vertical_angle), sizeof(float));

      save_file.close();
    }

    auto [cam_pos, cam_dir, _] = computeCameraCoords(window);

    if (mouse_released &&
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      mouse_pressed = true;
      mouse_released = false;
    }

    if (mouse_pressed &&
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
      mouse_pressed = false;
      mouse_released = true;

      g_fireball_position_data.push_back(cam_pos + glm::normalize(cam_dir));
      g_fireball_direction_data.push_back(glm::normalize(cam_dir));
    }

    /*** END EVENTS HANDLING ***/

    /*** WORLD UPDATE ***/

    for (size_t i = 0; i < g_fireball_position_data.size();) {
      if (glm::distance(g_fireball_position_data[i], cam_pos) > kFarRadius) {
        g_fireball_position_data.erase(g_fireball_position_data.begin() + i);
        g_fireball_direction_data.erase(g_fireball_direction_data.begin() + i);
      } else {
        ++i;
      }
    }

    for (size_t i = 0; i < g_fireball_position_data.size(); ++i) {
      for (size_t j = 0; j < g_enemies_position_data.size();) {
        if (glm::distance(g_fireball_position_data[i],
                          g_enemies_position_data[j]) < kCollisionRadius) {
          g_fireball_position_data.erase(g_fireball_position_data.begin() + i);
          g_fireball_direction_data.erase(g_fireball_direction_data.begin() +
                                          i);

          g_enemies_position_data.erase(g_enemies_position_data.begin() + j);
          g_enemies_rotation_data.erase(g_enemies_rotation_data.begin() + j);

          --i;

          break;
        } else {
          ++j;
        }
      }
    }

    auto current_time = glfwGetTime();

    if (current_time - start >= 5) {
      g_enemies_position_data.emplace_back(GetRandomPosition());
      g_enemies_rotation_data.push_back(GetRandomQuaternion());
      start = glfwGetTime();
    }

    for (size_t i = 0; i < g_fireball_position_data.size(); ++i) {
      g_fireball_position_data[i] +=
          g_fireball_direction_data[i] * kFireBallSpeed;
    }

    /*** END WORLD UPDATE ***/

    /*** DRAW ENEMIES ***/

    auto [projection_matrix, view_matrix] = computeMatricesFromInputs(window);
    glm::mat4 model_matrix = glm::mat4(1.0);
    glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

    glUseProgram(enemy_shader);
    glUniformMatrix4fv(matrix_id_enemy, 1, GL_FALSE, &mvp[0][0]);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(0,
                          3,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized
                          0,        // stride
                          nullptr   // array buffer offset
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glVertexAttribPointer(1,
                          3,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized
                          0,        // stride
                          nullptr   // array buffer offset
    );

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, enemies_position_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 g_enemies_position_data.size() * sizeof(glm::vec3),
                 g_enemies_position_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2,
                          3,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized
                          0,        // stride
                          nullptr   // array buffer offset
    );

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, enemies_rotation_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 g_enemies_rotation_data.size() * sizeof(glm::quat),
                 g_enemies_rotation_data.data(), GL_STREAM_DRAW);
    glVertexAttribPointer(3,
                          4,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized
                          0,        // stride
                          nullptr   // array buffer offset
    );
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6 * 3,
                          g_enemies_position_data.size());

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    /*** END DRAW ENEMIES ***/

    /*** DRAW FIREBALLS ***/

    glUseProgram(fireball_shader);
    glUniformMatrix4fv(matrix_id_fireball, 1, GL_FALSE, &mvp[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(fireball_texture_id, 0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, fireball_vertex_buffer);
    glVertexAttribPointer(0,        // attribute
                          3,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized
                          0,        // stride
                          nullptr   // array buffer offset
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, fireball_position_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 g_fireball_position_data.size() * sizeof(glm::vec3),
                 g_fireball_position_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1,        // attribute
                          3,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized
                          0,        // stride
                          nullptr   // array buffer offset
    );

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, fireball_uv_buffer);
    glVertexAttribPointer(2,        // attribute
                          2,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized
                          0,        // stride
                          nullptr   // array buffer offset
    );
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 0);

    glDrawArraysInstanced(GL_TRIANGLES, 0, fireball_vertices.size(),
                          g_fireball_position_data.size());

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    /*** END DRAW FIREBALLS ***/

    glfwSwapBuffers(window);

    glfwPollEvents();
  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

  glDeleteBuffers(1, &vertex_buffer);
  glDeleteVertexArrays(1, &vertex_array_id);
  glDeleteProgram(enemy_shader);

  glfwTerminate();

  return 0;
}
