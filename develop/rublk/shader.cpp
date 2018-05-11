#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "shader.h"

// --

class Shader::Impl
{
  unsigned int m_id;

public:
  // utility function for checking shader compilation/linking errors.
  bool checkCompileErrors(GLuint shader, std::string type)
  {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if(!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if(!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
      }
    }

    return success != 0;
  }

  void setId(unsigned int id)
  {
    m_id = id;
  }
  unsigned int getId() const
  {
    return m_id;
  }
};

// --

// constructor generates the shader on the fly
Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
  : m_pImpl(std::make_unique<Shader::Impl>())
{
  // 1. retrieve the vertex/fragment source code from filePath
  std::string vertexCode;
  std::string fragmentCode;
  std::string geometryCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;
  std::ifstream gShaderFile;
  // ensure ifstream objects can throw exceptions:
  vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
  gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
  try {
    // open files
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    std::stringstream vShaderStream, fShaderStream;
    // read file's buffer contents into streams
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    // close file handlers
    vShaderFile.close();
    fShaderFile.close();
    // convert stream into string
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
    // if geometry shader path is present, also load a geometry shader
    if(geometryPath != nullptr) {
      gShaderFile.open(geometryPath);
      std::stringstream gShaderStream;
      gShaderStream << gShaderFile.rdbuf();
      gShaderFile.close();
      geometryCode = gShaderStream.str();
    }
  }
  catch (std::ifstream::failure e) {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
  }
  const char* vShaderCode = vertexCode.c_str();
  const char * fShaderCode = fragmentCode.c_str();
  // 2. compile shaders
  unsigned int vertex, fragment;
  int success;
  char infoLog[512];

  // vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vShaderCode, NULL);
  glCompileShader(vertex);
  m_pImpl->checkCompileErrors(vertex, "VERTEX");

  // fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fShaderCode, NULL);
  glCompileShader(fragment);
  m_pImpl->checkCompileErrors(fragment, "FRAGMENT");

  // if geometry shader is given, compile geometry shader
  unsigned int geometry;
  if(geometryPath != nullptr) {
    const char * gShaderCode = geometryCode.c_str();
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &gShaderCode, NULL);
    glCompileShader(geometry);
    m_pImpl->checkCompileErrors(geometry, "GEOMETRY");
  }

  // shader Program
  m_pImpl->setId(glCreateProgram());
  glAttachShader(m_pImpl->getId(), vertex);
  glAttachShader(m_pImpl->getId(), fragment);
  if(geometryPath != nullptr) {
    glAttachShader(m_pImpl->getId(), geometry);
  }
  glLinkProgram(m_pImpl->getId());
  m_pImpl->checkCompileErrors(m_pImpl->getId(), "PROGRAM");

  // delete the shaders as they're linked into our program now and no longer necessery
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  if(geometryPath != nullptr) {
    glDeleteShader(geometry);
  }
}

Shader::~Shader()
{

}

Shader::Shader(Shader &&l) noexcept
  : m_pImpl(std::move(l.m_pImpl))
{
  l.m_pImpl = nullptr;
}

// activate the shader
// ------------------------------------------------------------------------
void Shader::use() const
{
  glUseProgram(m_pImpl->getId());
}
// utility uniform functions
// ------------------------------------------------------------------------
void Shader::setBool(const std::string &name, bool value) const
{
  glUniform1i(glGetUniformLocation(m_pImpl->getId(), name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string &name, int value) const
{
  glUniform1i(glGetUniformLocation(m_pImpl->getId(), name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string &name, float value) const
{
  glUniform1f(glGetUniformLocation(m_pImpl->getId(), name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
  glUniform2fv(glGetUniformLocation(m_pImpl->getId(), name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string &name, float x, float y) const
{
  glUniform2f(glGetUniformLocation(m_pImpl->getId(), name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
  glUniform3fv(glGetUniformLocation(m_pImpl->getId(), name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
  glUniform3f(glGetUniformLocation(m_pImpl->getId(), name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
  glUniform4fv(glGetUniformLocation(m_pImpl->getId(), name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string &name, float x, float y, float z, float w)
{
  glUniform4f(glGetUniformLocation(m_pImpl->getId(), name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
  glUniformMatrix2fv(glGetUniformLocation(m_pImpl->getId(), name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
  glUniformMatrix3fv(glGetUniformLocation(m_pImpl->getId(), name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
  glUniformMatrix4fv(glGetUniformLocation(m_pImpl->getId(), name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

