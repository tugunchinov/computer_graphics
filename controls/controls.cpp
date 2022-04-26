#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <controls.hpp>

constexpr float initial_fov = 45.0f;

constexpr float speed = 3.0F;
constexpr float mouseSpeed = 0.005F;

constexpr float x_center = 1024.f / 2;
constexpr float y_center = 768.f / 2;

glm::vec3 position = glm::vec3(0, 0, 5);
float horizontal_angle = 3.14f;
float vertical_angle = 0.0f;

std::tuple<glm::vec3, glm::vec3, glm::vec3>
computeCameraCoords(GLFWwindow *window) {
  static double lastTime = glfwGetTime();

  double currentTime = glfwGetTime();
  auto deltaTime = float(currentTime - lastTime);

  double x_pos{};
  double y_pos{};
  glfwGetCursorPos(window, &x_pos, &y_pos);
  glfwSetCursorPos(window, x_center, y_center);

  horizontal_angle += mouseSpeed * float(x_center - x_pos);
  vertical_angle += mouseSpeed * float(y_center - y_pos);

  // Direction: Spherical coordinates to Cartesian coordinates conversion
  glm::vec3 direction(std::cos(vertical_angle) * std::sin(horizontal_angle),
                      std::sin(vertical_angle),
                      std::cos(vertical_angle) * std::cos(horizontal_angle));

  // Right vector
  glm::vec3 right = glm::vec3(std::sin(horizontal_angle - 3.14f / 2.0f), 0,
                              std::cos(horizontal_angle - 3.14f / 2.0f));

  // Up vector
  glm::vec3 up = glm::cross(right, direction);

  // Move forward
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    position += direction * deltaTime * speed;
  }
  // Move backward
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    position -= direction * deltaTime * speed;
  }
  // Strafe right
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    position += right * deltaTime * speed;
  }
  // Strafe left
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    position -= right * deltaTime * speed;
  }

  lastTime = currentTime;

  return {position, direction, up};
}

std::pair<glm::mat4, glm::mat4> computeMatricesFromInputs(GLFWwindow *window) {
  auto [position, direction, up] = computeCameraCoords(window);

  glm::mat4 projection_matrix =
      glm::perspective(glm::radians(initial_fov), 16.0f / 9.0f, 0.1f, 100.0f);
  glm::mat4 view_matrix = glm::lookAt(position, position + direction, up);

  return {projection_matrix, view_matrix};
}