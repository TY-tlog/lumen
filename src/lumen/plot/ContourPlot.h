#pragma once

#include "plot/PlotItem.h"

#include <QColor>
#include <QPainter>
#include <QRectF>
#include <QString>

#include <memory>
#include <vector>

namespace lumen::data {
class Grid2D;
} // namespace lumen::data

namespace lumen::plot {

class CoordinateMapper;

/// A contour plot item that renders iso-lines from a Grid2D dataset.
///
/// Uses ContourAlgorithm (CONREC) to extract contour segments at specified
/// levels. Supports automatic level generation, level labels, and overlay
/// with Heatmap.
class ContourPlot : public PlotItem {
public:
    /// Construct a contour plot from a Grid2D.
    /// @throws std::invalid_argument if grid is null.
    explicit ContourPlot(std::shared_ptr<data::Grid2D> grid);

    // --- PlotItem overrides ---
    Type type() const override { return Type::Contour; }
    QRectF dataBounds() const override;
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    QColor primaryColor() const override { return lineColor_; }

    // --- Contour-specific API ---

    /// Automatically generate evenly-spaced levels from the data range.
    /// @param count Number of levels to generate (must be > 0).
    void setAutoLevels(int count);

    /// Manually set the iso-levels.
    void setLevels(std::vector<double> levels);

    /// Whether level labels are drawn at segment midpoints.
    void setLabelsVisible(bool visible);

    /// Line color for contour lines.
    void setLineColor(QColor color);

    /// Line width for contour lines.
    void setLineWidth(double width);

    /// Set the name of this plot item.
    void setName(QString name);

    /// Set visibility.
    void setVisible(bool visible);

    // Getters
    [[nodiscard]] const std::vector<double>& levels() const { return levels_; }
    [[nodiscard]] bool labelsVisible() const { return labelsVisible_; }
    [[nodiscard]] QColor lineColor() const { return lineColor_; }
    [[nodiscard]] double lineWidth() const { return lineWidth_; }
    [[nodiscard]] std::shared_ptr<data::Grid2D> grid() const { return grid_; }

private:
    /// Compute levels from data range.
    void computeAutoLevels();

    std::shared_ptr<data::Grid2D> grid_;
    std::vector<double> levels_;
    int autoLevelCount_ = 0; // 0 = manual levels
    bool labelsVisible_ = false;
    QColor lineColor_ = Qt::black;
    double lineWidth_ = 1.0;
    QString name_;
    bool visible_ = true;
};

} // namespace lumen::plot
