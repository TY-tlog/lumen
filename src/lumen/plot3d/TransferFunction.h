#pragma once

#include <QColor>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <vector>

namespace lumen::plot3d {

/// Transfer function for volume rendering.
///
/// Maps scalar values to RGBA colors via a set of control points.
/// Each control point has a value, color, and opacity.
/// toLUT() generates a 1D RGBA lookup texture.
class TransferFunction {
public:
    struct ControlPoint {
        double value = 0.0;   ///< Normalized value in [0, 1].
        QColor color{255, 255, 255};
        double opacity = 1.0; ///< Alpha in [0, 1].

        bool operator<(const ControlPoint& o) const { return value < o.value; }
    };

    TransferFunction() = default;

    /// Add a control point. Points are kept sorted by value.
    void addControlPoint(double value, QColor color, double opacity);

    /// Replace all control points.
    void setControlPoints(std::vector<ControlPoint> points);

    /// Access the current control points (sorted by value).
    [[nodiscard]] const std::vector<ControlPoint>& controlPoints() const
    {
        return points_;
    }

    /// Generate a 1D RGBA lookup table as a QImage (width=resolution, height=1).
    [[nodiscard]] QImage toLUT(int resolution = 256) const;

    /// Sample the transfer function at a normalized value t in [0,1].
    /// Returns (r, g, b, a) as a QColor with the alpha channel set.
    [[nodiscard]] QColor sample(double t) const;

    /// JSON persistence.
    [[nodiscard]] QJsonObject toJson() const;
    static TransferFunction fromJson(const QJsonObject& obj);

private:
    std::vector<ControlPoint> points_;
};

}  // namespace lumen::plot3d
