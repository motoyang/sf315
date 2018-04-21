#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

#include "singleton.h"
#include "shader_m.h"
#include "lamp.h"

static float vLamp[] = {
    // positions          // normals           // texture coords

    // 后绿
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.75f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.25f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.25f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.75f, 0.25f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.75f, 0.0f,

    // 前黑
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.50f,  0.25f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.75f,  0.25f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.75f,  0.50f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.75f,  0.50f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.50f,  0.50f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.50f,  0.25f,

    // 左橙
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.25f,  0.25f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.25f,  0.50f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.50f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.50f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.25f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.25f,  0.25f,

    // 右黑
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.75f,  0.25f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.75f,  0.50f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.50f,  0.50f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.50f,  0.50f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.50f,  0.25f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.75f,  0.25f,

    // 下白
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.25f,  0.25f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.50f,  0.25f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.50f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.50f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.25f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.25f,  0.25f,

    // 上黑
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.50f,  0.50f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.75f,  0.50f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.75f,  0.25f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.75f,  0.25f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.50f,  0.25f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.50f,  0.50f
};

class Lamp::Impl
{
public:
  glm::vec3 m_position;
  unsigned int m_vao, m_vbo;

  Impl()
  {}

  virtual ~Impl()
  {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
  }

  void initialize(const glm::vec3& pos)
  {
    m_position = pos;

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vLamp), vLamp, GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void render(const Shader& s)
  {
    glm::mat4 model = glm::mat4();
    model = glm::translate(model, m_position);
    model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube
    s.setMat4("model", model);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
  }
};

// --

Lamp::Lamp()
{
  m_pImpl = std::make_unique<Lamp::Impl>();
}

Lamp::~Lamp()
{

}

void Lamp::initialize(const glm::vec3& position)
{
  m_pImpl->initialize(position);
}

void Lamp::render(const Shader& s)
{
  m_pImpl->render(s);
}

glm::vec3 Lamp::getPosition() const
{
  return m_pImpl->m_position;
}


