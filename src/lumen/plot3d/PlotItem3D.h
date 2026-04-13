#pragma once

#include "BoundingBox3D.h"
#include "PbrMaterial.h"
#include "RenderContext.h"

#include <QString>

#include <optional>

namespace lumen::plot3d {

class ShaderProgram;
struct Ray;

class PlotItem3D {
public:
    enum class Type { Scatter3D, Surface3D, Volume, Streamlines, Isosurface };

    virtual ~PlotItem3D() = default;
    virtual Type type() const = 0;
    virtual QString name() const = 0;
    virtual BoundingBox3D dataBounds() const = 0;
    virtual void render(ShaderProgram& shader, const RenderContext& ctx) = 0;
    virtual std::optional<HitResult3D> hitTestRay(const Ray& ray,
                                                   double maxDist) const = 0;
    virtual bool isVisible() const = 0;
    virtual void invalidate() {}

    /// Optional PBR material. When set, Renderer3D uses the PBR shader.
    void setPbrMaterial(const PbrMaterial& mat) { pbrMaterial_ = mat; }
    void clearPbrMaterial() { pbrMaterial_.reset(); }
    [[nodiscard]] const std::optional<PbrMaterial>& pbrMaterial() const
    {
        return pbrMaterial_;
    }
    [[nodiscard]] bool hasPbrMaterial() const { return pbrMaterial_.has_value(); }

private:
    std::optional<PbrMaterial> pbrMaterial_;
};

}  // namespace lumen::plot3d
