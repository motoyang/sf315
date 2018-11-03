#include <iostream>
#include <memory>
#include <regex>
#include <iomanip>
#include <sstream>

#include <Corrade/Containers/Optional.h>
#include <Corrade/Utility/Arguments.h>

#include <Magnum/Timeline.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/Animable.h>
#include <Magnum/SceneGraph/AnimableGroup.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Object.h>

#include <Magnum/Ui/Anchor.h>
#include <Magnum/Ui/Button.h>
#include <Magnum/Ui/Label.h>
#include <Magnum/Ui/Plane.h>
#include <Magnum/Ui/UserInterface.h>
#include <Magnum/Ui/ValidatedInput.h>
#include <Magnum/Text/Alignment.h>

#include "cube.h"
#include "baseuiplane.h"
#include "MagnumRublk.h"

// --

MagnumRublk* g_app = nullptr;

// --

MagnumRublk::MagnumRublk(const Arguments& arguments)
  : Platform::Application{arguments, Configuration().setTitle("Magnum Textured Triangle Example")}
{
  using namespace Math::Literals;

  GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
  GL::Renderer::setClearColor(0x000000_rgbf);

  _cameraObject = std::make_unique<Object3D>(&_scene);
  setCameraDefaultPos();

  _camera = std::make_unique<SceneGraph::Camera3D>(*_cameraObject);
  _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
      .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 100.0f))
      .setViewport(GL::defaultFramebuffer.viewport().size());

  _rublk = std::make_unique<Rublk>(&_scene, &_drawables, &_animables, 3);

  Utility::Arguments args;
  args.addBooleanOption('c', "clean").setHelp("clean", "clean the rublk")
      .setHelp("Play on the rublk. Enjoy!")
      .parse(arguments.argc, arguments.argv);

  if (!args.isSet("clean")) {
    _rublk->confuse();
  }

  // 打开opengl的垂直同步。注意！显卡设置中的垂直同步也会影响这个值的设置。
  // ubuntu中设置nvidia垂直同步的方法如下：
  // $ sudo vim /etc/modprobe.d/nvidia-graphics-drivers.conf
  // 然后在最后一行添加:
  // options nvidia_drm modeset=1
  // 退出保存后，终端输入:
  // $ sudo update-initramfs -u
  // 重启，检查设置是否成功，显示1或Y就是打开了垂直同步：
  // $ sudo cat /sys/module/nvidia_drm/parameters/modeset
  CORRADE_INTERNAL_ASSERT_OUTPUT(setSwapInterval(1));

  // Create the UI
  _ui.emplace(Vector2{windowSize()}, windowSize(), Ui::mcssDarkStyleConfiguration(), "uiname");
  Interconnect::connect(*_ui, &Ui::UserInterface::inputWidgetFocused, *this, &MagnumRublk::startTextInput);
  Interconnect::connect(*_ui, &Ui::UserInterface::inputWidgetBlurred, *this, &MagnumRublk::stopTextInput);

  // Base UI plane
  _baseUiPlane.emplace(*_ui);
  Interconnect::connect(_baseUiPlane->_degrees, &Ui::Input::valueChanged, *this, &MagnumRublk::enableApplyButton);
  Interconnect::connect(_baseUiPlane->_apply, &Ui::Button::tapped, *this, &MagnumRublk::apply);
  Interconnect::connect(_baseUiPlane->_D, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_d, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_U, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_u, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_B, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_b, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_F, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_f, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_R, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_r, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_L, &Ui::Button::tapped, *this, &MagnumRublk::roll);
  Interconnect::connect(_baseUiPlane->_l, &Ui::Button::tapped, *this, &MagnumRublk::roll);

  apply();

  // start timeline
  _timeline.start();
  g_app = this;
}

void MagnumRublk::setFps(Float f)
{
  std::stringstream out;
  out << "FPS: " << std::setprecision(3) << f;
  _baseUiPlane->_fps.setText(out.str());
}

void MagnumRublk::enableApplyButton(const std::string &)
{
  _baseUiPlane->_apply.setEnabled(Ui::ValidatedInput::allValid({_baseUiPlane->_degrees}));
}

void MagnumRublk::apply()
{
  Int d = std::stoi(_baseUiPlane->_degrees.value());
  _rublk->setRollSpeed(d);
}

void MagnumRublk::roll()
{
  if (_baseUiPlane->_F.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('F');
  } else if (_baseUiPlane->_f.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('f');
  } else if (_baseUiPlane->_B.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('B');
  } else if (_baseUiPlane->_b.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('b');
  } else if (_baseUiPlane->_L.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('L');
  } else if (_baseUiPlane->_l.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('l');
  } else if (_baseUiPlane->_R.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('R');
  } else if (_baseUiPlane->_r.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('r');
  } else if (_baseUiPlane->_U.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('U');
  } else if (_baseUiPlane->_u.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('u');
  } else if (_baseUiPlane->_D.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('D');
  } else if (_baseUiPlane->_d.flags() & Ui::WidgetFlag::Active) {
    _rublk->pushEvent('d');
  }
}

void MagnumRublk::drawEvent()
{
  _animables.step(_timeline.previousFrameTime(), _timeline.previousFrameDuration());
  GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);
  
  GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
  _camera->draw(_drawables);
  GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

  /* Draw the UI */
  GL::Renderer::enable(GL::Renderer::Feature::Blending);
  GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
  _ui->draw();
  GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);
  GL::Renderer::disable(GL::Renderer::Feature::Blending);

  swapBuffers();
  redraw();
  _timeline.nextFrame();
}

void MagnumRublk::viewportEvent(const Vector2i& size) {
  GL::defaultFramebuffer.setViewport({{}, size});
}

void MagnumRublk::mousePressEvent(MouseEvent& event)
{
  _ui->handlePressEvent(event.position());

  if (event.button() != MouseEvent::Button::Left) {
    return;
  }
  _previousMousePosition = _mousePressPosition = event.position();
  event.setAccepted();
}

void MagnumRublk::mouseMoveEvent(MouseMoveEvent& event)
{
  if(_ui->handleMoveEvent(event.position())) {
    // UI handles it
    return;
  }

  if (!(event.buttons() & MouseMoveEvent::Button::Left)) {
    return;
  }

  const Vector2 delta = 3.0f*
      Vector2{event.position() - _previousMousePosition}/
      Vector2{GL::defaultFramebuffer.viewport().size()};

  _cameraObject->rotate(Rad{-delta.y()}, _cameraObject->transformation().right().normalized())
    .rotateY(Rad{-delta.x()});

  // 每50次，对cameraObject的Matrix做正交化，消除浮点数计算的误差累积
  static Int count = 0;
  if (++count > 50) {
    orthonormalizeOnObject(_cameraObject.get());
    count = 0;
  }

  _previousMousePosition = event.position();
  event.setAccepted();
  redraw();
}

void MagnumRublk::mouseReleaseEvent(MouseEvent &event)
{
  _ui->handleReleaseEvent(event.position());
}

void MagnumRublk::keyPressEvent(KeyEvent &event)
{
  // If an input is focused, pass the events only to the UI
  if(isTextInputActive() && _ui->focusedInputWidget()) {
    _ui->focusedInputWidget()->handleKeyPress(event);
  }
}

void MagnumRublk::keyReleaseEvent(KeyEvent& event)
{
  bool shift(event.modifiers() & KeyEvent::Modifier::Shift);

  switch (event.key()) {
  case KeyEvent::Key::F:
    _rublk->pushEvent(shift? 'F': 'f');
    break;
  case KeyEvent::Key::B:
    _rublk->pushEvent(shift? 'B': 'b');
    break;
  case KeyEvent::Key::L:
    _rublk->pushEvent(shift? 'L': 'l');
    break;
  case KeyEvent::Key::R:
    _rublk->pushEvent(shift? 'R': 'r');
    break;
  case KeyEvent::Key::U:
    _rublk->pushEvent(shift? 'U': 'u');
    break;
  case KeyEvent::Key::D:
    _rublk->pushEvent(shift? 'D': 'd');
    break;
  case KeyEvent::Key::Space:
    _cameraObject->rotate(180.0_degf, _cameraObject->transformation().up().normalized());
    break;
  case KeyEvent::Key::Enter: {
    bool alt(event.modifiers() & KeyEvent::Modifier::Alt);
    if (alt) {
      SDL_Window* w = window();
      bool fullScreen(SDL_GetWindowFlags(w) & SDL_WINDOW_FULLSCREEN);
      SDL_SetWindowFullscreen(w, fullScreen? 0: SDL_WINDOW_FULLSCREEN);
    } else {
      setCameraDefaultPos();
    }
  }
    break;

  default:
    return;
  }
}

void MagnumRublk::textInputEvent(TextInputEvent &event)
{
  if(isTextInputActive() && _ui->focusedInputWidget()) {
    _ui->focusedInputWidget()->handleTextInput(event);
  }
}

void MagnumRublk::setCameraDefaultPos()
{
  _cameraObject->resetTransformation()
      .translate(Vector3::zAxis(10.0f))
      .rotateX(-30.0_degf)
      .rotateY(30.0_degf);
}

// --

MAGNUM_APPLICATION_MAIN(MagnumRublk)
