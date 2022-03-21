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

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  GLuint vertex_array_id;
  glGenVertexArrays(1, &vertex_array_id);
  glBindVertexArray(vertex_array_id);

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

  GLuint shader =
      LoadShaders("./shader/vertex_shader", "./shader/fragment_shader");

  GLint matrix_id = glGetUniformLocation(shader, "MVP");

  float x = 3.0f;
  float y = 0.0f;
  float z = -0.5f;

  glm::mat4 proj =
      glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
  glm::mat4 view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 0.0f, 1.0f));
  glm::mat4 model = glm::mat4(1.0f);

  glm::mat4 MVP = proj * view * model;

  auto start = std::chrono::system_clock::now();

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    glUseProgram(shader);
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glfwSwapBuffers(window);

    std::this_thread::sleep_for(50ms);

    const float radius = 3.0f;
    x = std::sin(static_cast<float>(glfwGetTime())) * radius;
    y = std::cos(static_cast<float>(glfwGetTime())) * radius;

    view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, 0.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f));

    MVP = proj * view * model;

    glfwPollEvents();
  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

  glDeleteBuffers(1, &vertex_buffer);
  glDeleteVertexArrays(1, &vertex_array_id);
  glDeleteProgram(shader);

  glfwTerminate();

  return 0;
}
