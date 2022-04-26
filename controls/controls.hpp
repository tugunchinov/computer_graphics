#pragma once

#include <tuple>
#include <utility>

#include <glm/glm.hpp>

std::pair<glm::mat4, glm::mat4> computeMatricesFromInputs(GLFWwindow *window);

std::tuple<glm::vec3, glm::vec3, glm::vec3>
computeCameraCoords(GLFWwindow *window);
