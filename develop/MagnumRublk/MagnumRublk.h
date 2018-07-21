#ifndef MAGNUMRUBLK_H
#define MAGNUMRUBLK_H

using namespace Magnum;

// --

class MagnumRublk
    : public Platform::Application
    , public Interconnect::Receiver
{
  friend class Rublk;
  friend class Cube;

public:
  explicit MagnumRublk(const Arguments& arguments);

  void setFps(Float f);
  void enableApplyButton(const std::string&);
  void apply();
  void roll();

private:
  void viewportEvent(const Vector2i& size) override;
  void drawEvent() override;
  void mousePressEvent(MouseEvent& event) override;
  void mouseMoveEvent(MouseMoveEvent& event) override;
  void mouseReleaseEvent(MouseEvent& event) override;
  void keyPressEvent(KeyEvent& event) override;
  void keyReleaseEvent(KeyEvent& event) override;
  void textInputEvent(TextInputEvent& event) override;

  void setCameraDefaultPos();

  Scene3D _scene;
  SceneGraph::DrawableGroup3D _drawables;
  SceneGraph::AnimableGroup3D _animables;
  std::unique_ptr<Object3D> _cameraObject;
  std::unique_ptr<SceneGraph::Camera3D> _camera;
  std::unique_ptr<Rublk> _rublk;

  Timeline _timeline;
  Vector2i _previousMousePosition, _mousePressPosition;

  // UI
  Containers::Optional<Ui::UserInterface> _ui;
  Containers::Optional<BaseUiPlane> _baseUiPlane;
};

extern MagnumRublk* g_app;

#endif // MAGNUMRUBLK_H
