#include <vector>
#include <queue>
#include <list>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "learnopengl/shader_m.h"
#include "cube.h"

// png图片的y坐标是从上到下，x坐标是从左到右

// 上黄   下白    前蓝    后绿
// UuDd, FfBb, LlRr
// 0.0f, 0.25f, 0.50f, 0.75f, 1.0f

// 左橙   右红    内黑
// 0.0f, 0.25f, 0.50f, 0.75f, 1.0f

static float v27[][288] = {
  {
    // id = 0, 上黑下白，前黑后绿，左橙右黑
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
  }, {
    // id = 1, 上黑下白，前黑后黑，左橙右黑
    // positions          // normals           // texture coords

    // 后黑
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.50f,  0.25f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.75f,  0.25f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.75f,  0.50f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.75f,  0.50f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.50f,  0.50f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.50f,  0.25f,

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
  }, {
    // 上黄   下白    前蓝    后绿
    // UuDd, FfBb, LlRr
    // 0.0f, 0.25f, 0.50f, 0.75f, 1.0f

    // 左橙   右红    内黑
    // 0.0f, 0.25f, 0.50f, 0.75f, 1.0f

    // id = 2, 上黑下白，前蓝后绿，左橙右黑
    // positions          // normals           // texture coords

    // 后绿
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.75f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.25f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.25f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.75f, 0.25f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.75f, 0.0f,

    // 前蓝
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.50f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.75f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.75f,  0.25f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.75f,  0.25f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.50f,  0.25f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.50f,  0.0f,

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
  }, {
    // 上黄   下白    前蓝    后绿
    // UuDd, FfBb, LlRr
    // 0.0f, 0.25f, 0.50f, 0.75f, 1.0f

    // 左橙   右红    内黑
    // 0.0f, 0.25f, 0.50f, 0.75f, 1.0f

    // id = 3, 上黑下黑，前黑后绿，左橙右黑
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

    // 下黑
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.50f,  0.50f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.75f,  0.50f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.75f,  0.25f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.75f,  0.25f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.50f,  0.25f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.50f,  0.50f,

    // 上黑
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.50f,  0.50f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.75f,  0.50f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.75f,  0.25f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.75f,  0.25f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.50f,  0.25f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.50f,  0.50f
  }
};

// --

Cube::Cube(Rublk* r, int id, const float *vertices, const glm::vec3 &position)
  : m_rublk(r)
  , m_id(id)
  , m_vertices(vertices)
  , m_position(position)
  , m_angle(-1)
{
  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(v27[0]), m_vertices, GL_STATIC_DRAW);

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

Cube::~Cube()
{
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
}

void Cube::render(const Shader& s)
{
  if (m_angle == 0) {
    m_rublk->taskFinished();
    m_angle = -1;

//    m_position = glm::vec3(m_model * glm::vec4(m_position, 1.0));
  }
  if (m_angle > 0) {
    m_model = glm::rotate(m_model, glm::radians(1.0f), m_axis);
    --m_angle;
  }
  //  m_model = glm::translate(m_model, m_position);
  s.setMat4("model", glm::translate(m_model, m_position));

  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

bool Cube::isTop() const
{
  bool r = false;
  if (m_position.y > 1.0) {
    r = true;
  }
  return r;
}

bool Cube::isRight() const
{
  bool r = false;
  if (m_position.x > 1.0) {
    r = true;
  }
  return r;
}

void Cube::setPosition(const glm::vec3& v)
{
  m_position = v;
}

glm::vec3 Cube::getPosition() const
{
//  return m_position;
  glm::vec3 p = glm::vec3(m_model * glm::vec4(m_position, 1.0f));
  return p;
//  return m_position;
}

void Cube::rotate(int degree, const glm::vec3 axis)
{
  m_angle = degree;
  m_axis = glm::vec3(glm::inverse(m_model) * glm::vec4(axis, 1.0f));
  m_rublk->taskStart();
}

// --

void Rublk::eventHandler()
{
  // 如果仍然有任务在执行，就不做事件处理
  if (m_taskCount) {
    return;
  }

  if (m_events.size() == 0) {
    return;
  }

  char c = m_events.front(); m_events.pop();
  switch (c) {
  case 'r':
    std::for_each(m_cubes.begin(), m_cubes.end(), [](Cube& c) {
      glm::vec3 p = c.getPosition();
      if (p.x > 1.0) {
        c.rotate(90, glm::vec3(1.0f, 0.0f, 0.0f));
      }
    });
    break;
  case 'R':
    std::for_each(m_cubes.begin(), m_cubes.end(), [](Cube& c) {
      glm::vec3 p = c.getPosition();
      if (p.x > 1.0) {
        c.rotate(90, glm::vec3(-1.0f, 0.0f, 0.0f));
      }
    });
    break;
  case 't':
    std::for_each(m_cubes.begin(), m_cubes.end(), [](Cube& c) {
      glm::vec3 p = c.getPosition();
      if (p.y > 1.0) {
        c.rotate(90, glm::vec3(0.0f, 1.0f, 0.0f));
      }
    });
    break;
  default:
    break;
  }
}

Rublk::Rublk(int rank)
  : m_rank(rank)
  , m_taskCount(0)
{
  // rublk中心在长宽高的第几个cube
  const float center = rank / 2.0f - 0.5f;
  // cube的长宽高为1.0f，两个cube中心的距离如下：
  const float gap = 1.05f;
\
  // 很重要！
  // 提前分配足够的空间，避免emplace_back过程中的重新分配。
  // 如果重新分配空间，会析构已经分配的cube，因为cube的析构中有m_vao, m_vbo的删除，会导致使用已经删除的buffer。
  m_cubes.reserve(rank * rank * rank);

  for (int i = 0; i < rank; ++i) {
    for (int j = 0; j < rank; ++j) {
      for (int k = 0; k < rank; ++k) {
        // 三维魔方，数量是rank的立方
        int id = i * rank * rank + j * rank + k;
        glm::vec3 v(i - center, j - center, k - center);
//        m_cubes.emplace_back(id, v27[id], v * grid);
        m_cubes.emplace_back(this, id, v27[3], v * gap);
      }
    }
  }
}

void Rublk::render(const Shader &s)
{
  eventHandler();

  std::for_each(m_cubes.begin(), m_cubes.end(), [s](Cube& c) {
    c.render(s);
  });
}

void Rublk::roate()
{
  std::vector<Cube*> r;
  std::for_each(m_cubes.begin(), m_cubes.end(), [&r](Cube& c) {
    if (c.isTop()) {
      r.push_back(&c);

      glm::mat4 model;
      float angle = 20.0f;
      model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
      glm::vec3 pos = glm::vec3(model * glm::vec4(c.getPosition(), 1.0));
      c.setPosition(pos);
    }
  });
}

void Rublk::sendEvent(const char c)
{
//  static char s_c = 0;
//  if (s_c == c) return;

  m_events.push(c);
//  s_c = c;
}

void Rublk::taskStart()
{
  ++m_taskCount;
}

void Rublk::taskFinished()
{
  --m_taskCount;
}
