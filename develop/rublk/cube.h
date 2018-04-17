#ifndef CUBE_H
#define CUBE_H

#define degreesToRadians(x)   ((x) * 3.141592f / 180.0f)

class Rublk;

class Cube
{
  Rublk* m_rublk;
  int m_id;
  const float* m_vertices;
  glm::vec3 m_position;
  glm::mat4 m_model;
  unsigned int m_vao, m_vbo;

  int m_angle;
  glm::vec3 m_axis;

public:
  Cube(Rublk* r, int id, const float* vertices, const glm::vec3& position);
  virtual ~Cube();

  void render(const Shader& s);
  bool isTop() const;
  bool isRight() const;
  void setPosition(const glm::vec3& v);
  glm::vec3 getPosition() const;
  void rotate(int degree, const glm::vec3 axis);

};

class Rublk
{
  int m_rank;
  int m_taskCount;
  std::vector<Cube> m_cubes;
  std::queue<char> m_events;

  void eventHandler();

public:
  Rublk(int rank);
  void render(const Shader& s);
  void roate();
  void sendEvent(const char c);
  void taskStart();
  void taskFinished();
};

#endif // CUBE_H
