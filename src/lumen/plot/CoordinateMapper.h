#pragma once

#include <QPointF>
#include <QRectF>

#include <utility>

namespace lumen::plot {

/// Bidirectional mapping between data coordinates and pixel coordinates.
///
/// Y axis is inverted: higher Y data values map to lower pixel Y values.
class CoordinateMapper {
public:
    /// Construct with data range and pixel rect.
    CoordinateMapper(double xMin, double xMax, double yMin, double yMax,
                     QRectF pixelRect);

    /// Default constructor — identity mapping on unit square.
    CoordinateMapper();

    /// Map data coordinates to pixel coordinates.
    [[nodiscard]] QPointF dataToPixel(double x, double y) const;

    /// Map pixel coordinates to data coordinates.
    [[nodiscard]] std::pair<double, double> pixelToData(QPointF pixel) const;

    /// Update the data range.
    void setDataRange(double xMin, double xMax, double yMin, double yMax);

    /// Update the pixel rect.
    void setPixelRect(QRectF rect);

    [[nodiscard]] double xMin() const { return xMin_; }
    [[nodiscard]] double xMax() const { return xMax_; }
    [[nodiscard]] double yMin() const { return yMin_; }
    [[nodiscard]] double yMax() const { return yMax_; }
    [[nodiscard]] QRectF pixelRect() const { return pixelRect_; }

private:
    double xMin_{0.0};
    double xMax_{1.0};
    double yMin_{0.0};
    double yMax_{1.0};
    QRectF pixelRect_{0.0, 0.0, 1.0, 1.0};
};

} // namespace lumen::plot
