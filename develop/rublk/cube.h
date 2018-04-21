#ifndef CUBE_H
#define CUBE_H

class Rublk
{
  friend class Singleton<Rublk>;

  class Impl;
  std::unique_ptr<Impl> m_pImpl;

protected:
  Rublk();

public:
  virtual ~Rublk();

  bool initialize(int rank, unsigned int skin);
  void confuse();

  void render(const Shader& s);
  void pushEvent(const char c);
  void taskStart();
  void taskFinished();
};

#endif // CUBE_H
