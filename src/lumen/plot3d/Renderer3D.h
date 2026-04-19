#pragma once

#include "PbrMaterial.h"
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

    /// Whether the PBR shader compiled successfully.
    [[nodiscard]] bool hasPbrShader() const { return pbrInitialized_; }

    /// Select the appropriate shader for the given item.
    /// Returns the PBR shader if the item has a PBR material, else Phong.
    [[nodiscard]] ShaderProgram& shaderForItem(const PlotItem3D& item);

private:
    void renderItem(PlotItem3D& item, const RenderContext& ctx);
    void setupPbrUniforms(const PbrMaterial& mat, const RenderContext& ctx);

    ShaderProgram phongShader_;
    ShaderProgram pointShader_;
    ShaderProgram pbrShader_;
    bool initialized_ = false;
    bool pointInitialized_ = false;
    bool pbrInitialized_ = false;
};

}  // namespace lumen::plot3d
