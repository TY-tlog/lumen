#include "plot/Heatmap.h"

#include "data/Grid2D.h"
#include "plot/CoordinateMapper.h"

#include <QImage>
#include <QPointF>
#include <QRectF>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace lumen::plot {

Heatmap::Heatmap(std::shared_ptr<data::Grid2D> grid, Colormap colormap)
    : grid_(std::move(grid))
    , colormap_(std::move(colormap))
{
    if (!grid_) {
        throw std::invalid_argument("Heatmap: grid must not be null");
    }
}

QRectF Heatmap::dataBounds() const
{
    if (!grid_) {
        return {};
    }

    auto dims = grid_->dimensions();
    // dims[0] = dimX (columns), dims[1] = dimY (rows)
    std::size_t cols = dims[0].length;
    std::size_t rows = dims[1].length;

    if (cols == 0 || rows == 0) {
        return {};
    }

    double xStart = dims[0].coordinates.valueAt(0);
    double xEnd = dims[0].coordinates.valueAt(cols - 1);
    double yStart = dims[1].coordinates.valueAt(0);
    double yEnd = dims[1].coordinates.valueAt(rows - 1);

    double xMin = std::min(xStart, xEnd);
    double xMax = std::max(xStart, xEnd);
    double yMin = std::min(yStart, yEnd);
    double yMax = std::max(yStart, yEnd);

    return QRectF(QPointF(xMin, yMin), QPointF(xMax, yMax));
}

void Heatmap::paint(QPainter* painter, const CoordinateMapper& mapper,
                    const QRectF& plotArea) const
{
    if (!visible_ || !grid_) {
        return;
    }

    // GPU path deferred to T9. For now, always use CPU.
    renderCpu(painter, mapper, plotArea);
}

void Heatmap::renderCpu(QPainter* painter, const CoordinateMapper& mapper,
                        const QRectF& plotArea) const
{
    auto shape = grid_->shape();
    // shape[0] = dimX.length (cols), shape[1] = dimY.length (rows)
    std::size_t cols = shape[0];
    std::size_t rows = shape[1];

    if (cols == 0 || rows == 0) {
        return;
    }

    // Compute value range
    double vMin = valueMin_;
    double vMax = valueMax_;
    if (autoRange_) {
        vMin = std::numeric_limits<double>::max();
        vMax = std::numeric_limits<double>::lowest();
        for (std::size_t r = 0; r < rows; ++r) {
            for (std::size_t c = 0; c < cols; ++c) {
                double v = grid_->valueAt({c, r});
                if (std::isnan(v)) {
                    continue;
                }
                vMin = std::min(vMin, v);
                vMax = std::max(vMax, v);
            }
        }
        if (vMin >= vMax) {
            vMin = 0;
            vMax = 1;
        }
    }

    // Get grid coordinate ranges from dimensions
    auto dims = grid_->dimensions();
    double xStart = dims[0].coordinates.valueAt(0);
    double xEnd = dims[0].coordinates.valueAt(cols - 1);
    double yStart = dims[1].coordinates.valueAt(0);
    double yEnd = dims[1].coordinates.valueAt(rows - 1);

    // Create QImage at grid resolution.
    // Image pixel (c, r) maps to grid value at column c, row r.
    // Row 0 of the image is the top; we map yEnd (highest y) to image row 0.
    QImage image(static_cast<int>(cols), static_cast<int>(rows),
                 QImage::Format_ARGB32);

    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < cols; ++c) {
            double v = grid_->valueAt({c, r});
            double t = (vMax > vMin) ? (v - vMin) / (vMax - vMin) : 0.5;
            t = std::clamp(t, 0.0, 1.0);
            QColor color = colormap_.sample(t);
            if (opacity_ < 1.0) {
                color.setAlphaF(static_cast<float>(opacity_));
            }
            // Image row 0 = highest y value (yEnd), last row = lowest y (yStart)
            auto imageRow = static_cast<int>(rows - 1 - r);
            image.setPixelColor(static_cast<int>(c), imageRow, color);
        }
    }

    // Map grid corners to pixel space
    QPointF topLeft = mapper.dataToPixel(xStart, yEnd);
    QPointF bottomRight = mapper.dataToPixel(xEnd, yStart);
    QRectF targetRect(topLeft, bottomRight);

    // Draw the image scaled to the target rect
    painter->save();
    painter->setClipRect(plotArea);
    if (interp_ == Interpolation::Bilinear) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    }
    painter->drawImage(targetRect, image);
    painter->restore();
}

QColor Heatmap::primaryColor() const
{
    return colormap_.sample(0.5);
}

Heatmap::RenderPath Heatmap::activeRenderPath() const
{
    auto shape = grid_->shape();
    return (shape[0] * shape[1] > 1024 * 1024) ? RenderPath::GPU : RenderPath::CPU;
}

void Heatmap::setColormap(Colormap cmap)
{
    colormap_ = std::move(cmap);
}

void Heatmap::setValueRange(double min, double max)
{
    valueMin_ = min;
    valueMax_ = max;
    autoRange_ = false;
}

void Heatmap::setAutoValueRange()
{
    autoRange_ = true;
}

void Heatmap::setInterpolation(Interpolation interp)
{
    interp_ = interp;
}

void Heatmap::setOpacity(double opacity)
{
    opacity_ = std::clamp(opacity, 0.0, 1.0);
}

void Heatmap::setName(QString name)
{
    name_ = std::move(name);
}

void Heatmap::setVisible(bool visible)
{
    visible_ = visible;
}

} // namespace lumen::plot
