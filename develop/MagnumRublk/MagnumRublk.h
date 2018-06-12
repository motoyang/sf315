#ifndef MAGNUMRUBLK_H
#define MAGNUMRUBLK_H

using namespace Magnum;

class MagnumRublk: public Platform::Application
{
  friend class Rublk;
  friend class Cube;

public:
  explicit MagnumRublk(const Arguments& arguments);

private:
  void drawEvent() override;
  void mousePressEvent(MouseEvent& event) override;
  void mouseMoveEvent(MouseMoveEvent& event) override;
  void keyReleaseEvent(KeyEvent& event) override;

  void setCameraPos();

  Scene3D _scene;
  SceneGraph::DrawableGroup3D _drawables;
  SceneGraph::AnimableGroup3D _animables;
  std::unique_ptr<Object3D> _cameraObject;
  std::unique_ptr<SceneGraph::Camera3D> _camera;
  std::unique_ptr<Rublk> _rublk;

  Timeline _timeline;
  Vector2i _previousMousePosition, _mousePressPosition;
};

extern MagnumRublk* g_app;

#endif // MAGNUMRUBLK_H
