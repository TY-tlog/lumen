#pragma once

#include "RenderContext.h"
#include "ShaderProgram.h"

#include <QSize>

namespace lumen::plot3d {

class Scene3D;
class Camera;
class PlotItem3D;

class Renderer3D {
public:
    /// Compile shaders. Returns false on failure.
    bool initialize();

    /// Render the entire scene from the camera's viewpoint.
    void render(Scene3D& scene, Camera& camera, QSize viewport);

    [[nodiscard]] bool isInitialized() const { return initialized_; }

private:
    void renderItem(PlotItem3D& item, ShaderProgram& shader,
                    const RenderContext& ctx);

    ShaderProgram phongShader_;
    bool initialized_ = false;
};

}  // namespace lumen::plot3d
