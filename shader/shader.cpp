#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <GL/glew.h>

#include <shader.hpp>

GLuint LoadShaders(const char *vertex_file_path,
                   const char *fragment_file_path) {

  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if (VertexShaderStream.is_open()) {
    std::stringstream sstr;
    sstr << VertexShaderStream.rdbuf();
    VertexShaderCode = sstr.str();
    VertexShaderStream.close();
  } else {
    std::cerr << "Impossible to open" << vertex_file_path
              << "Are you in the right directory?" << std::endl;
    return 1;
  }

  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if (FragmentShaderStream.is_open()) {
    std::stringstream sstr;
    sstr << FragmentShaderStream.rdbuf();
    FragmentShaderCode = sstr.str();
    FragmentShaderStream.close();
  }

  // Compile Vertex Shader
  std::cout << "Compiling shader: " << vertex_file_path << std::endl;
  char const *VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  GLint Result = GL_FALSE;
  int InfoLogLength;
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr,
                       &VertexShaderErrorMessage[0]);
    std::cerr << &VertexShaderErrorMessage[0] << std::endl;
  }

  // Compile Fragment Shader
  std::cout << "Compiling shader: " << fragment_file_path << std::endl;
  char const *FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr,
                       &FragmentShaderErrorMessage[0]);
    std::cerr << &FragmentShaderErrorMessage[0] << std::endl;
  }

  // Link the program
  std::cout << "Linking program" << std::endl;
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
    glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr,
                        &ProgramErrorMessage[0]);
    std::cerr << &ProgramErrorMessage[0] << std::endl;
  }

  glDetachShader(ProgramID, VertexShaderID);
  glDetachShader(ProgramID, FragmentShaderID);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}
