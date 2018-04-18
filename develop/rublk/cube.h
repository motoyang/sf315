#ifndef CUBE_H
#define CUBE_H

class Rublk;

class Cube
{
  Rublk* m_rublk;
  int m_id;
  glm::vec3 m_position;
  glm::mat4 m_model;
  unsigned int m_vao, m_vbo;

  int m_angle;
  glm::vec3 m_axis;

public:
  Cube(Rublk* r, int id, const float* vertices, const glm::vec3& position);
  virtual ~Cube();

  void render(const Shader& s);
  glm::vec3 getPosition() const;
  void rotate(int degree, const glm::vec3& axis);
  void changeModel(int degree, const glm::vec3& axis);
};

class Rublk
{
  int m_rank;
  int m_taskCount;
  unsigned int m_skin;
  std::vector<Cube> m_cubes;
  std::queue<char> m_events;

  void eventHandler();

public:
  Rublk(int rank, unsigned int skin);
  void confuse();

  void render(const Shader& s);
  void pushEvent(const char c);
  void taskStart();
  void taskFinished();
};

#endif // CUBE_H
