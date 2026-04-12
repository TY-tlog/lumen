#pragma once

#include "plot/Colormap.h"
#include "plot/PlotItem.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QRectF>
#include <QString>

#include <memory>

namespace lumen::data {
class Grid2D;
} // namespace lumen::data

namespace lumen::plot {

class CoordinateMapper;

/// A heatmap plot item that renders a Grid2D dataset as a colored image.
///
/// Maps grid values through a Colormap to produce a raster image.
/// Supports nearest-neighbor and bilinear interpolation modes.
/// Uses CPU rendering for grids up to 1M cells; will fall back to
/// GPU path (T9) for larger grids.
class Heatmap : public PlotItem {
public:
    enum class Interpolation { Nearest, Bilinear };
    enum class RenderPath { CPU, GPU };

    /// Construct a heatmap from a Grid2D and a colormap.
    /// @throws std::invalid_argument if grid is null.
    Heatmap(std::shared_ptr<data::Grid2D> grid, Colormap colormap);

    // --- PlotItem overrides ---
    Type type() const override { return Type::Heatmap; }
    QRectF dataBounds() const override;
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    QColor primaryColor() const override;

    // Setters
    void setColormap(Colormap cmap);
    void setValueRange(double min, double max);
    void setAutoValueRange();
    void setInterpolation(Interpolation interp);
    void setOpacity(double opacity);
    void setName(QString name);
    void setVisible(bool visible);

    // Getters
    [[nodiscard]] const Colormap& colormap() const { return colormap_; }
    [[nodiscard]] double valueMin() const { return valueMin_; }
    [[nodiscard]] double valueMax() const { return valueMax_; }
    [[nodiscard]] Interpolation interpolation() const { return interp_; }
    [[nodiscard]] double opacity() const { return opacity_; }
    [[nodiscard]] RenderPath activeRenderPath() const;
    [[nodiscard]] std::shared_ptr<data::Grid2D> grid() const { return grid_; }

private:
    void renderCpu(QPainter* painter, const CoordinateMapper& mapper,
                   const QRectF& plotArea) const;

    std::shared_ptr<data::Grid2D> grid_;
    Colormap colormap_;
    double valueMin_ = 0.0;
    double valueMax_ = 1.0;
    bool autoRange_ = true;
    Interpolation interp_ = Interpolation::Nearest;
    double opacity_ = 1.0;
    QString name_;
    bool visible_ = true;
};

} // namespace lumen::plot
