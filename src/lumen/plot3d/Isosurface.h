#pragma once

#include "MarchingCubes.h"
#include "PlotItem3D.h"

#include <QColor>
#include <QString>

#include <memory>
#include <vector>

namespace lumen::data {
class Volume3D;
}

namespace lumen::plot3d {

/// Isosurface plot item. Extracts and renders one or more isosurfaces
/// from a Volume3D using Marching Cubes.
class Isosurface : public PlotItem3D {
public:
    explicit Isosurface(std::shared_ptr<data::Volume3D> volume,
                        QString name = {});

    ~Isosurface() override;

    Type type() const override { return Type::Isosurface; }
    BoundingBox3D dataBounds() const override;
    void render(ShaderProgram& shader, const RenderContext& ctx) override;
    std::optional<HitResult3D> hitTestRay(const Ray& ray,
                                           double maxDist) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    void invalidate() override;

    // Iso values
    void setIsoValues(std::vector<double> values);
    [[nodiscard]] const std::vector<double>& isoValues() const
    {
        return isoValues_;
    }

    // Colors for each iso value
    void setColors(std::vector<QColor> colors);
    [[nodiscard]] const std::vector<QColor>& colors() const
    {
        return colors_;
    }

    // Accessors
    void setName(QString name);
    void setVisible(bool v);

    /// Access computed meshes (one per iso value, for testing).
    [[nodiscard]] const std::vector<MarchingCubes::Mesh>& meshes() const;

    /// Total triangle count across all iso surfaces.
    [[nodiscard]] std::size_t totalTriangleCount() const;

private:
    void buildMeshes() const;

    std::shared_ptr<data::Volume3D> volume_;
    std::vector<double> isoValues_;
    std::vector<QColor> colors_;
    QString name_;
    bool visible_ = true;

    mutable bool meshesDirty_ = true;
    mutable std::vector<MarchingCubes::Mesh> meshes_;
};

}  // namespace lumen::plot3d
