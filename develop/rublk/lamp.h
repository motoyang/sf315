#ifndef LAMP_H
#define LAMP_H

class Lamp
{
  friend class Singleton<Lamp>;

  class Impl;
  std::unique_ptr<Impl> m_pImpl;

protected:
  Lamp();

public:
  virtual ~Lamp();

  void initialize(const glm::vec3& position);
  void render(const Shader& s);
  glm::vec3 getPosition() const;
};

#endif // LAMP_H
