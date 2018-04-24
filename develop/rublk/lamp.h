#ifndef LAMP_H
#define LAMP_H

class Lamp
{
  class Impl;
  std::unique_ptr<Impl> m_pImpl;

public:
  Lamp(const glm::vec3& pos);
  virtual ~Lamp();
  Lamp(Lamp&& l);


  void render(const Shader& s);
  glm::vec3 getPosition() const;
};

#endif // LAMP_H
