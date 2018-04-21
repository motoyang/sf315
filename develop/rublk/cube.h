#ifndef CUBE_H
#define CUBE_H

#define ORIGIN_CAMERA_POSITION glm::vec3(3.0f, 4.0f, 5.0f)

class Rublk
{
  friend class Singleton<Rublk>;

  class Impl;
  std::unique_ptr<Impl> m_pImpl;

protected:
  Rublk();

public:
  virtual ~Rublk();

  bool initialize(int rank, unsigned int diffuseMap, unsigned int specularMap);
  void confuse();

  void render(const Shader& s);
  void pushEvent(const char c);
  void taskStart();
  void taskFinished();
};

#endif // CUBE_H
