#include <chrono>
#include <iostream>
#include <random>
#include <thread>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.hpp>

using namespace std::chrono_literals;

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(1024, 768, "Triangle", nullptr, nullptr);
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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  GLuint vertex_array_id;
  glGenVertexArrays(1, &vertex_array_id);
  glBindVertexArray(vertex_array_id);

  static const GLfloat g_vertex_buffer_data[] = {
      // first triangle
      -1.0f, -1.0f, 0.0f, // vertex 1
      1.0f, -1.0f, 0.0f,  // vertex 2
      0.0f, 1.0f, 0.0f,   // vertex 3

      // second triangle
      1.0f, 1.0f, 0.0f,  // vertex 1
      -1.0f, 1.0f, 0.0f, // vertex 2
      0.0f, -1.0f, 0.0f, // vertex 3
  };

  GLuint vertex_buffer;
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
               g_vertex_buffer_data, GL_STATIC_DRAW);

  GLuint shader_1 =
      LoadShaders("./shader/vertex_shader_1", "./shader/fragment_shader_1");

  GLuint shader_2 =
      LoadShaders("./shader/vertex_shader_2", "./shader/fragment_shader_2");

  GLint matrix_id = glGetUniformLocation(shader_1, "MVP");

  float x = 0.0f;
  float y = 0.0f;
  float z = 3.0f;

  glm::mat4 proj =
      glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
  glm::mat4 view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 model = glm::mat4(1.0f);

  glm::mat4 MVP = proj * view * model;

  std::mt19937 rng(std::random_device{}());
  std::normal_distribution<float> dist{0.0f, 0.1f};

  auto start = std::chrono::system_clock::now();

  do {
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(0,
                          3,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized
                          0,        // stride
                          nullptr   // array buffer offset
    );

    glUseProgram(shader_1);
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glUseProgram(shader_2);
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 3, 3);

    glDisableVertexAttribArray(0);

    glfwSwapBuffers(window);

    std::this_thread::sleep_for(100ms);

    auto dur = std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now() - start)
                   .count();

    if (dur < 3) {
      z += 0.3f;  // up
    } else if (dur < 6) {
      z -= 0.3f;  // down
    } else if (dur < 8) {
      x += dist(rng);  // quake
      y += dist(rng);
      z += dist(rng);
    } else {
      start = std::chrono::system_clock::now();
    }

    view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, 0.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f));

    MVP = proj * view * model;

    glfwPollEvents();
  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

  glDeleteBuffers(1, &vertex_buffer);
  glDeleteVertexArrays(1, &vertex_array_id);
  glDeleteProgram(shader_1);
  glDeleteProgram(shader_2);

  glfwTerminate();

  return 0;
}
