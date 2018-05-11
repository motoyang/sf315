#ifndef LAMP_H
#define LAMP_H

class Lamp
{
  class Impl;
  std::unique_ptr<Impl> m_pImpl;

public:
  Lamp(const glm::vec3& pos);
  virtual ~Lamp();

  // 如果类的成员变量中有uniqu_ptr类型，就要提供移动构造函数，不要拷贝构造函数。
  // 因为uniqu_ptr没有拷贝构造函数，只有移动构造函数。
  Lamp(Lamp&& l) noexcept;
  Lamp(const Lamp& l) = delete;

  void render(const Shader& s);
  glm::vec3 getPosition() const;
};

#endif // LAMP_H
