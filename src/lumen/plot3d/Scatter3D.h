#pragma once

#include "PlotItem3D.h"
#include "SpatialGrid3D.h"

#include <QColor>
#include <QString>

#include <memory>
#include <vector>

namespace lumen::data {
class Rank1Dataset;
}

namespace lumen::plot3d {

/// 3D scatter plot item. Renders data points as GL_POINTS with uniform color.
///
/// Hit-testing uses ray-sphere intersection per point. When the point count
/// exceeds 10,000, a SpatialGrid3D is built lazily to accelerate queries.
class Scatter3D : public PlotItem3D {
public:
    enum class MarkerShape3D { Sphere, Cube, Tetrahedron };

    Scatter3D(std::shared_ptr<data::Rank1Dataset> xData,
              std::shared_ptr<data::Rank1Dataset> yData,
              std::shared_ptr<data::Rank1Dataset> zData,
              QColor color = Qt::blue, QString name = {});

    ~Scatter3D() override;

    Type type() const override { return Type::Scatter3D; }
    BoundingBox3D dataBounds() const override;
    void render(ShaderProgram& shader, const RenderContext& ctx) override;
    std::optional<HitResult3D> hitTestRay(const Ray& ray,
                                           double maxDist) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    void invalidate() override;

    // Setters
    void setMarkerShape(MarkerShape3D shape);
    void setMarkerSize(float worldUnits);
    void setColor(QColor color);
    void setName(QString name);
    void setVisible(bool v);

    // Getters
    [[nodiscard]] MarkerShape3D markerShape() const { return shape_; }
    [[nodiscard]] float markerSize() const { return markerSize_; }
    [[nodiscard]] QColor color() const { return color_; }

    /// Direct access to cached point positions (built on first access).
    [[nodiscard]] const std::vector<QVector3D>& pointPositions() const;

private:
    void buildPositionCache() const;
    void buildSpatialGrid() const;

    static constexpr std::size_t kSpatialGridThreshold = 10000;

    std::shared_ptr<data::Rank1Dataset> xData_;
    std::shared_ptr<data::Rank1Dataset> yData_;
    std::shared_ptr<data::Rank1Dataset> zData_;
    MarkerShape3D shape_ = MarkerShape3D::Sphere;
    float markerSize_ = 0.05f;
    QColor color_;
    QString name_;
    bool visible_ = true;

    // Lazy-built caches.
    mutable bool positionsDirty_ = true;
    mutable std::vector<QVector3D> positions_;
    mutable std::unique_ptr<SpatialGrid3D> spatialGrid_;
};

}  // namespace lumen::plot3d
