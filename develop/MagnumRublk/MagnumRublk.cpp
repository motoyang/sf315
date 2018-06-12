#include <iostream>
#include <memory>

#include <Corrade/Containers/Optional.h>
#include <Corrade/Utility/Arguments.h>

#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Object.h>

#include "cube.h"
#include "MagnumRublk.h"

using namespace Magnum;

MagnumRublk* g_app = nullptr;

MagnumRublk::MagnumRublk(const Arguments& arguments)
  : Platform::Application{arguments, Configuration{}.setTitle("Magnum Textured Triangle Example")}
{
  using namespace Math::Literals;

  GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
  GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

  _rublk = std::make_unique<Rublk>(&_scene, &_drawables, 3);

  _cameraObject = std::make_unique<Object3D>(&_scene);
  setCameraPos();

  _camera = std::make_unique<SceneGraph::Camera3D>(*_cameraObject);
  _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
      .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 100.0f))
      .setViewport(GL::defaultFramebuffer.viewport().size());

  Utility::Arguments args;
  args.addBooleanOption('c', "confuse").setHelp("confuse", "confuse the rublk")
      .setHelp("Play on the rublk. Enjoy!")
      .parse(arguments.argc, arguments.argv);

  if (args.isSet("confuse")) {
    _rublk->confuse();
  }

  g_app = this;
}

void MagnumRublk::drawEvent()
{
  GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

  _camera->draw(_drawables);

  swapBuffers();
}

void MagnumRublk::mousePressEvent(MouseEvent& event)
{
  if (event.button() != MouseEvent::Button::Left) {
    return;
  }

  _previousMousePosition = _mousePressPosition = event.position();
  event.setAccepted();
}

void MagnumRublk::mouseMoveEvent(MouseMoveEvent& event)
{
  if (!(event.buttons() & MouseMoveEvent::Button::Left)) {
    return;
  }

  const Vector2 delta = 3.0f*
      Vector2{event.position() - _previousMousePosition}/
      Vector2{GL::defaultFramebuffer.viewport().size()};

  (*_cameraObject)
      .rotate(Rad{-delta.y()}, _cameraObject->transformation().right().normalized())
      .rotateY(Rad{-delta.x()});

  _previousMousePosition = event.position();
  event.setAccepted();
  redraw();
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
    case KeyEvent::Key::Enter:
      setCameraPos();
      break;

    default:
      return;
    }

    redraw();
}

void MagnumRublk::setCameraPos()
{
  _cameraObject->resetTransformation()
      .translate(Vector3::zAxis(10.0f))
      .rotateX(-30.0_degf)
      .rotateY(30.0_degf);
}

MAGNUM_APPLICATION_MAIN(MagnumRublk)
