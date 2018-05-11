#ifndef SHADER_H
#define SHADER_H

class Shader
{
  class Impl;
  std::unique_ptr<Impl> m_pImpl;

public:
//  unsigned int ID;
  Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
  virtual ~Shader();

  // 如果类的成员变量中有uniqu_ptr类型，就要提供移动构造函数，不要拷贝构造函数。
  // 因为uniqu_ptr没有拷贝构造函数，只有移动构造函数。
  Shader(Shader&& l) noexcept;
  Shader(const Shader& l) = delete;

  bool Compile() const;
  void use() const;
  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec2(const std::string &name, const glm::vec2 &value) const;
  void setVec2(const std::string &name, float x, float y) const;
  void setVec3(const std::string &name, const glm::vec3 &value) const;
  void setVec3(const std::string &name, float x, float y, float z) const;
  void setVec4(const std::string &name, const glm::vec4 &value) const;
  void setVec4(const std::string &name, float x, float y, float z, float w);
  void setMat2(const std::string &name, const glm::mat2 &mat) const;
  void setMat3(const std::string &name, const glm::mat3 &mat) const;
  void setMat4(const std::string &name, const glm::mat4 &mat) const;
};
#endif

