/*
    This file is part of Magnum.

    Original authors — credit is appreciated but not required:

        2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 —
            Vladimír Vondruš <mosra@centrum.cz>

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to
    the public domain. We make this dedication for the benefit of the public
    at large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all
    present and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <memory>
#include <utility>
#include <vector>
#include <string>

#include <Corrade/Utility/Assert.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Directory.h>

#include <Magnum/Image.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Primitives/Axis.h>
#include <Magnum/Primitives/Capsule.h>
#include <Magnum/Primitives/Circle.h>
#include <Magnum/Primitives/Crosshair.h>
#include <Magnum/Primitives/Cone.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Cylinder.h>
#include <Magnum/Primitives/Grid.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Primitives/Line.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Shaders/MeshVisualizer.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Shaders/VertexColor.h>
#include <Magnum/Trade/MeshData2D.h>
#include <Magnum/Trade/MeshData3D.h>
#include <Magnum/Trade/AbstractImageConverter.h>
#include <Magnum/Trade/AbstractImporter.h>

namespace Magnum { namespace Examples {

using namespace Magnum::Math::Literals;

namespace {
    const auto BaseColor = 0x2f83cc_rgbf;
    const auto OutlineColor = 0xdcdcdc_rgbf;
    const auto Projection2D = Matrix3::projection({3.0f, 3.0f});
    const auto Projection3D = Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.001f, 100.0f);
    const auto Transformation2D = Matrix3::rotation(13.2_degf);
    const auto Transformation3D = Matrix4::translation(Vector3::zAxis(-6.0f))*
                                  Matrix4::rotationY(-10.82_degf)*
                                  Matrix4::rotationX(24.37_degf)*
                                  Matrix4::rotationZ(18.3_degf);
}

// --

class PrimitivesDraw {
public:
  virtual void draw() = 0;
};

// --

class SoldDraw: public PrimitivesDraw {
    GL::Buffer _indexBuffer, _vertexBuffer;
    GL::Mesh _mesh;
    Shaders::Phong _shader;
    Shaders::MeshVisualizer _wireframe{Shaders::MeshVisualizer::Flag::Wireframe};

public:
  explicit SoldDraw() {}
  explicit SoldDraw(const Trade::MeshData3D& data);

  SoldDraw(SoldDraw&&) = delete;
  SoldDraw(const SoldDraw&) = delete;
  SoldDraw& operator=(SoldDraw&&) = delete;
  SoldDraw& operator=(const SoldDraw&) = delete;

  void draw() override;
};

SoldDraw::SoldDraw(const Trade::MeshData3D& data)
{
    _vertexBuffer.setData(MeshTools::interleave(data.positions(0), data.normals(0)), GL::BufferUsage::StaticDraw);
    _mesh.setPrimitive(data.primitive())
        .addVertexBuffer(_vertexBuffer, 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{});
    if (data.isIndexed()) {
        Containers::Array<char> indexData;
        MeshIndexType indexType;
        UnsignedInt indexStart, indexEnd;
        std::tie(indexData, indexType, indexStart, indexEnd) = MeshTools::compressIndices(data.indices());
        _indexBuffer.setData(indexData, GL::BufferUsage::StaticDraw);

        _mesh.setCount(data.indices().size())
            .setIndexBuffer(_indexBuffer, 0, indexType, indexStart, indexEnd);
    } else {
        _mesh.setCount(data.positions(0).size());
    }

    _shader.setAmbientColor(0x22272e_rgbf)
        .setDiffuseColor(BaseColor)
        .setSpecularColor(0x000000_rgbf)
        .setLightPosition({5.0f, 5.0f, 7.0f})
        .setProjectionMatrix(Projection3D)
        .setTransformationMatrix(Transformation3D)
        .setNormalMatrix(Transformation3D.rotationScaling());
    _wireframe.setColor(0x000000_rgbaf)
        .setWireframeColor(OutlineColor)
        .setViewportSize(Vector2(GL::defaultFramebuffer.viewport().size()))
        .setTransformationProjectionMatrix(Projection3D*Transformation3D);
}

void SoldDraw::draw()
{
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    _mesh.draw(_shader);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);
    _mesh.draw(_wireframe);
    // GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);
}

// --

class D2Draw: public PrimitivesDraw {
    GL::Buffer _indexBuffer, _vertexBuffer;
    GL::Mesh _mesh;
    Shaders::VertexColor2D _shader;

public:
  explicit D2Draw() {}
  explicit D2Draw(const Trade::MeshData2D& data);

  void draw() override;
};

D2Draw::D2Draw(const Trade::MeshData2D& data)
{
    _vertexBuffer.setData(MeshTools::interleave(data.positions(0), data.colors(0)), GL::BufferUsage::StaticDraw);
    _indexBuffer.setData(data.indices(), GL::BufferUsage::StaticDraw);

    _mesh.addVertexBuffer(_vertexBuffer, 0, Shaders::VertexColor2D::Position{},
                          Shaders::VertexColor2D::Color{Shaders::VertexColor2D::Color::Components::Four})
        .setIndexBuffer(_indexBuffer, 0, GL::MeshIndexType::UnsignedInt)
        .setCount(data.indices().size())
        .setPrimitive(data.primitive());

    _shader.setTransformationProjectionMatrix(Projection2D*Transformation2D);
}

void D2Draw::draw()
{
    _mesh.draw(_shader);
}

// --

class D3Draw: public PrimitivesDraw {
    GL::Buffer _indexBuffer, _vertexBuffer;
    GL::Mesh _mesh;
    Shaders::VertexColor3D _shader;

public:
  explicit D3Draw() {}
  explicit D3Draw(const Trade::MeshData3D& data);
  void draw() override;
};

D3Draw::D3Draw(const Trade::MeshData3D& data)
{
    _vertexBuffer.setData(MeshTools::interleave(data.positions(0), data.colors(0)), GL::BufferUsage::StaticDraw);
    _indexBuffer.setData(data.indices(), GL::BufferUsage::StaticDraw);

    _mesh.addVertexBuffer(_vertexBuffer, 0, Shaders::VertexColor3D::Position{},
                          Shaders::VertexColor3D::Color{Shaders::VertexColor3D::Color::Components::Four})
                .setIndexBuffer(_indexBuffer, 0, GL::MeshIndexType::UnsignedInt)
                .setCount(data.indices().size())
                .setPrimitive(data.primitive());
    _shader.setTransformationProjectionMatrix(Projection3D * Transformation3D);
}

void D3Draw::draw()
{
    _mesh.draw(_shader);
}

// --

class Flat2DDraw: public PrimitivesDraw {
    GL::Buffer _indexBuffer, _vertexBuffer;
    GL::Mesh _mesh;
    Shaders::Flat2D _shader;

public:
  explicit Flat2DDraw() {}
  explicit Flat2DDraw(const Trade::MeshData2D& data);
  void draw() override;
};

Flat2DDraw::Flat2DDraw(const Trade::MeshData2D& data)
{
    _vertexBuffer.setData(data.positions(0), GL::BufferUsage::StaticDraw);
    _mesh.addVertexBuffer(_vertexBuffer, 0, Shaders::Flat2D::Position{})
        .setPrimitive(data.primitive());
    if (data.isIndexed()) {
        _indexBuffer.setData(data.indices(), GL::BufferUsage::StaticDraw);
        _mesh.setIndexBuffer(_indexBuffer, 0, GL::MeshIndexType::UnsignedInt)
            .setCount(data.indices().size());
    } else {
        _mesh.setCount(data.positions(0).size());
    }
    _shader.setColor(OutlineColor)
        .setTransformationProjectionMatrix(Projection2D*Transformation2D);
}

void Flat2DDraw::draw()
{
    _mesh.draw(_shader);
}

// --

class Flat3DDraw: public PrimitivesDraw {
    GL::Buffer _indexBuffer, _vertexBuffer;
    GL::Mesh _mesh;
    Shaders::Flat3D _shader;

public:
  explicit Flat3DDraw() {}
  explicit Flat3DDraw(const Trade::MeshData3D& data);
  void draw() override;
};

Flat3DDraw::Flat3DDraw(const Trade::MeshData3D& data)
{
    _vertexBuffer.setData(data.positions(0), GL::BufferUsage::StaticDraw);
    _mesh.addVertexBuffer(_vertexBuffer, 0, Shaders::Flat3D::Position{})
        .setPrimitive(data.primitive());
    if (data.isIndexed()) {
        _indexBuffer.setData(data.indices(), GL::BufferUsage::StaticDraw);
        _mesh.setIndexBuffer(_indexBuffer, 0, GL::MeshIndexType::UnsignedInt)
            .setCount(data.indices().size());
    } else {
        _mesh.setCount(data.positions(0).size());
    }
    _shader.setColor(OutlineColor)
        .setTransformationProjectionMatrix(Projection3D*Transformation3D);
}

void Flat3DDraw::draw()
{
    _mesh.draw(_shader);
}

// --

class VisualizerDraw: public PrimitivesDraw {
    GL::Buffer _indexBuffer, _vertexBuffer;
    GL::Mesh _mesh;
    Shaders::MeshVisualizer _shader{Shaders::MeshVisualizer::Flag::Wireframe};

public:
  explicit VisualizerDraw() {}
  explicit VisualizerDraw(const Trade::MeshData2D& data);
  explicit VisualizerDraw(const Trade::MeshData3D& data);

  void draw() override;
};

VisualizerDraw::VisualizerDraw(const Trade::MeshData2D& data)
{
    _vertexBuffer.setData(data.positions(0), GL::BufferUsage::StaticDraw);
    _mesh.setPrimitive(data.primitive())
        .addVertexBuffer(_vertexBuffer, 0, Shaders::MeshVisualizer::Position{Shaders::MeshVisualizer::Position::Components::Two});
    if (data.isIndexed()) {
        Containers::Array<char> indexData;
        MeshIndexType indexType;
        UnsignedInt indexStart, indexEnd;
        std::tie(indexData, indexType, indexStart, indexEnd) = MeshTools::compressIndices(data.indices());
        _indexBuffer.setData(indexData, GL::BufferUsage::StaticDraw);

        _mesh.setCount(data.indices().size())
            .setIndexBuffer(_indexBuffer, 0, indexType, indexStart, indexEnd);
    } else {
        _mesh.setCount(data.positions(0).size());
    }

    const Matrix3 projection = Projection2D*Transformation2D;
    _shader.setColor(BaseColor)
        .setWireframeColor(OutlineColor)
        .setViewportSize(Vector2(GL::defaultFramebuffer.viewport().size()))
        .setTransformationProjectionMatrix(Matrix4{
            {projection[0], 0.0f},
            {projection[1], 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {{projection[2].xy(), 0.0f}, 1.0f}});
}

VisualizerDraw::VisualizerDraw(const Trade::MeshData3D& data)
{
      _vertexBuffer.setData(MeshTools::interleave(data.positions(0), data.normals(0)), GL::BufferUsage::StaticDraw);
    _mesh.setPrimitive(data.primitive())
        .addVertexBuffer(_vertexBuffer, 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{});
    if (data.isIndexed()) {
        Containers::Array<char> indexData;
        MeshIndexType indexType;
        UnsignedInt indexStart, indexEnd;
        std::tie(indexData, indexType, indexStart, indexEnd) = MeshTools::compressIndices(data.indices());
        _indexBuffer.setData(indexData, GL::BufferUsage::StaticDraw);

        _mesh.setCount(data.indices().size())
            .setIndexBuffer(_indexBuffer, 0, indexType, indexStart, indexEnd);
    } else {
        _mesh.setCount(data.positions(0).size());
    }

    _shader.setColor(BaseColor)
        .setWireframeColor(OutlineColor)
        .setViewportSize(Vector2(GL::defaultFramebuffer.viewport().size()))
        .setTransformationProjectionMatrix(Projection3D*Transformation3D);
}

void VisualizerDraw::draw()
{
    _mesh.draw(_shader);
}

// --

class PrimitivesExample: public Platform::Application {
    public:
        explicit PrimitivesExample(const Arguments& arguments);

    private:
        void drawEvent() override;
        void keyReleaseEvent(KeyEvent& event) override;

        std::pair<std::string, std::unique_ptr<PrimitivesDraw>> createPrimitives(int index);
        void savePng();

        std::unique_ptr<PrimitivesDraw> _primitivesDraw;
        std::string _name;
        Int _index = 0;
};

PrimitivesExample::PrimitivesExample(const Arguments& arguments):
    Platform::Application{arguments, Configuration{}.setTitle("Magnum Primitives Example")}
{
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    std::tie(_name, _primitivesDraw) = createPrimitives(_index++);
}

void PrimitivesExample::drawEvent() 
{
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    _primitivesDraw->draw();

    swapBuffers();
}

void PrimitivesExample::savePng()
{
    PluginManager::Manager<Trade::AbstractImageConverter> converterManager;
    std::unique_ptr<Trade::AbstractImageConverter> converter = converterManager.loadAndInstantiate("PngImageConverter");
    if(!converter) {
        Error() << "Cannot load image converter plugin";
    }

    Vector2i imageSize{GL::defaultFramebuffer.viewport().size()};

    GL::Renderbuffer color;
    color.setStorage(GL::RenderbufferFormat::RGBA8, imageSize);
    GL::Framebuffer framebuffer{{{}, imageSize}};
    framebuffer.attachRenderbuffer(GL::Framebuffer::ColorAttachment{0}, color);

    GL::AbstractFramebuffer::blit(GL::defaultFramebuffer, framebuffer, framebuffer.viewport(), GL::FramebufferBlit::Color);
    Image2D result = framebuffer.read(framebuffer.viewport(), {PixelFormat::RGBA8Unorm});
    converter->exportToFile(result, Utility::Directory::join("./", "primitives2-" + _name));
}

std::pair<std::string, std::unique_ptr<PrimitivesDraw>> PrimitivesExample::createPrimitives(int index)
{
    static std::vector<std::function<std::pair<std::string, std::unique_ptr<PrimitivesDraw>>()>> fn2 = {
        [](){return std::make_pair(std::string("axis2D"), std::make_unique<D2Draw>(Primitives::axis2D()));},
        [](){return std::make_pair(std::string("axis3D"), std::make_unique<D3Draw>(Primitives::axis3D()));},
        [](){return std::make_pair(std::string("capsule2DWireframe"), std::make_unique<Flat2DDraw>(Primitives::capsule2DWireframe(8, 1, 0.75f)));},
        [](){return std::make_pair(std::string("circle2DWireframe"), std::make_unique<Flat2DDraw>(Primitives::circle2DWireframe(32)));},
        [](){return std::make_pair(std::string("crosshair2D"), std::make_unique<Flat2DDraw>(Primitives::crosshair2D()));},
        [](){return std::make_pair(std::string("line2D"), std::make_unique<Flat2DDraw>(Primitives::line2D()));},
        [](){return std::make_pair(std::string("squareWireframe"), std::make_unique<Flat2DDraw>(Primitives::squareWireframe()));},
        [](){return std::make_pair(std::string("capsule3DWireframe"), std::make_unique<Flat3DDraw>(Primitives::capsule3DWireframe(8, 1, 16, 1.0f)));},
        [](){return std::make_pair(std::string("circle3DWireframe"), std::make_unique<Flat3DDraw>(Primitives::circle3DWireframe(32)));},
        [](){return std::make_pair(std::string("crosshair3D"), std::make_unique<Flat3DDraw>(Primitives::crosshair3D()));},
        [](){return std::make_pair(std::string("coneWireframe"), std::make_unique<Flat3DDraw>(Primitives::coneWireframe(32, 1.25f)));},
        [](){return std::make_pair(std::string("cubeWireframe"), std::make_unique<Flat3DDraw>(Primitives::cubeWireframe()));},
        [](){return std::make_pair(std::string("cylinderWireframe"), std::make_unique<Flat3DDraw>(Primitives::cylinderWireframe(1, 32, 1.0f)));},
        [](){return std::make_pair(std::string("grid3DWireframe"), std::make_unique<Flat3DDraw>(Primitives::grid3DWireframe({5, 3})));},
        [](){return std::make_pair(std::string("line3D"), std::make_unique<Flat3DDraw>(Primitives::line3D()));},
        [](){return std::make_pair(std::string("planeWireframe"), std::make_unique<Flat3DDraw>(Primitives::planeWireframe()));},
        [](){return std::make_pair(std::string("uvSphereWireframe"), std::make_unique<Flat3DDraw>(Primitives::uvSphereWireframe(16, 32)));},
        [](){return std::make_pair(std::string("circle2DSolid"), std::make_unique<VisualizerDraw>(Primitives::circle2DSolid(16)));},
        [](){return std::make_pair(std::string("squareSolid"), std::make_unique<VisualizerDraw>(Primitives::squareSolid()));},
        [](){return std::make_pair(std::string("capsule3DSolid"), std::make_unique<VisualizerDraw>(Primitives::capsule3DSolid(4, 1, 12, 0.75f)));},
        [](){return std::make_pair(std::string("circle3DSolid"), std::make_unique<VisualizerDraw>(Primitives::circle3DSolid(16)));},
        [](){return std::make_pair(std::string("capsule3DSolid"), std::make_unique<SoldDraw>(Primitives::capsule3DSolid(4, 1, 12, 0.75f)));},
        [](){return std::make_pair(std::string("circle3DSolid"), std::make_unique<SoldDraw>(Primitives::circle3DSolid(16)));},
        [](){return std::make_pair(std::string("coneSolid"), std::make_unique<SoldDraw>(Primitives::coneSolid(1, 12, 1.25f, Primitives::ConeFlag::CapEnd)));},
        [](){return std::make_pair(std::string("cubeSolid"), std::make_unique<SoldDraw>(Primitives::cubeSolid()));},
        [](){return std::make_pair(std::string("cylinderSolid"), std::make_unique<SoldDraw>(Primitives::cylinderSolid(1, 12, 1.0f, Primitives::CylinderFlag::CapEnds)));},
        [](){return std::make_pair(std::string("grid3DSolid"), std::make_unique<SoldDraw>(Primitives::grid3DSolid({5, 3})));},
        [](){return std::make_pair(std::string("icosphereSolid"), std::make_unique<SoldDraw>(Primitives::icosphereSolid(1)));},
        [](){return std::make_pair(std::string("planeSolid"), std::make_unique<SoldDraw>(Primitives::planeSolid()));},
        [](){return std::make_pair(std::string("uvSphereSolid"), std::make_unique<SoldDraw>(Primitives::uvSphereSolid(8, 16)));}
    };
    return fn2.at(index % fn2.size())();
}

void PrimitivesExample::keyReleaseEvent(KeyEvent& event)
{
  switch (event.key()) {
  case KeyEvent::Key::Space:
    std::tie(_name, _primitivesDraw) =  createPrimitives(_index++);
    redraw();
    break;
    
  case KeyEvent::Key::S:
    savePng();
    break;

  default:
    return;
  }
}

}}

MAGNUM_APPLICATION_MAIN(Magnum::Examples::PrimitivesExample)
