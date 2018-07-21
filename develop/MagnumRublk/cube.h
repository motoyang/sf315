#ifndef CUBE_H
#define CUBE_H

using namespace Magnum;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

void orthonormalizeOnObject(Object3D* o);

class Rublk: public Object3D, public SceneGraph::Drawable3D, public SceneGraph::Animable3D
{
  class Impl;
  std::unique_ptr<Impl> m_pImpl;

  void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
  void animationStep(Float time, Float delta) override;

protected:

public:
  Rublk(Scene3D *scene, SceneGraph::DrawableGroup3D *drawables,
        Magnum::SceneGraph::AnimableGroup3D *animables, int rank);
  virtual ~Rublk();

  // Rublk是单体设计，所以就不要下面四个函数
  Rublk(Rublk&& r) noexcept = delete;
  Rublk& operator =(Rublk&& r) noexcept = delete;
  Rublk(const Rublk& r) = delete;
  Rublk& operator =(const Rublk& r) = delete;

  void confuse();

  void pushEvent(const char c);
  void taskStart();
  void taskFinished();

  void setRollSpeed(Int degrees);
  Int rollSpeed() const;
};

#endif // CUBE_H
