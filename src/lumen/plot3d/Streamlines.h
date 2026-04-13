#pragma once

#include "PlotItem3D.h"

#include <QColor>
#include <QString>
#include <QVector3D>

#include <memory>
#include <vector>

namespace lumen::data {
class Volume3D;
}

namespace lumen::plot {
class Colormap;
}

namespace lumen::plot3d {

/// 3D streamline visualization using RK4 integration.
///
/// Takes three Volume3D fields (vx, vy, vz) representing a vector field.
/// Integrates streamlines from seed points using 4th-order Runge-Kutta.
/// Uses trilinear interpolation to sample the vector field.
class Streamlines : public PlotItem3D {
public:
    Streamlines(std::shared_ptr<data::Volume3D> vx,
                std::shared_ptr<data::Volume3D> vy,
                std::shared_ptr<data::Volume3D> vz,
                QString name = {});

    ~Streamlines() override;

    Type type() const override { return Type::Streamlines; }
    BoundingBox3D dataBounds() const override;
    void render(ShaderProgram& shader, const RenderContext& ctx) override;
    std::optional<HitResult3D> hitTestRay(const Ray& ray,
                                           double maxDist) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    void invalidate() override;

    // Seed points
    void setSeedPoints(std::vector<QVector3D> seeds);
    void setSeedGrid(int resolution);  ///< Create grid of seeds in the domain.
    [[nodiscard]] const std::vector<QVector3D>& seedPoints() const
    {
        return seeds_;
    }

    // Integration parameters
    void setIntegrationStep(float h);
    [[nodiscard]] float integrationStep() const { return stepSize_; }

    void setMaxSteps(int steps);
    [[nodiscard]] int maxSteps() const { return maxSteps_; }

    // Appearance
    void setColor(QColor color);
    [[nodiscard]] QColor color() const { return color_; }

    void setColormap(std::shared_ptr<plot::Colormap> cmap);

    void setName(QString name);
    void setVisible(bool v);

    /// A single streamline as a polyline.
    struct Line {
        std::vector<QVector3D> points;
        std::vector<float> magnitudes;  ///< Velocity magnitude at each point.
    };

    /// Access computed streamlines (for testing).
    [[nodiscard]] const std::vector<Line>& lines() const;

    /// Trilinear interpolation of the vector field at position p.
    [[nodiscard]] QVector3D sampleField(const QVector3D& p) const;

private:
    void buildStreamlines() const;

    /// RK4 integration step.
    [[nodiscard]] QVector3D rk4Step(const QVector3D& p, float h) const;

    /// Check if a point is inside the volume domain.
    [[nodiscard]] bool isInsideDomain(const QVector3D& p) const;

    std::shared_ptr<data::Volume3D> vx_;
    std::shared_ptr<data::Volume3D> vy_;
    std::shared_ptr<data::Volume3D> vz_;
    std::shared_ptr<plot::Colormap> colormap_;

    std::vector<QVector3D> seeds_;
    float stepSize_ = 0.05f;
    int maxSteps_ = 500;
    QColor color_{Qt::cyan};
    QString name_;
    bool visible_ = true;

    mutable bool linesDirty_ = true;
    mutable std::vector<Line> lines_;
    mutable BoundingBox3D domainBounds_;
};

}  // namespace lumen::plot3d
