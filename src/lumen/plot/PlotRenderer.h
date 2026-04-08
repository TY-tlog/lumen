#pragma once

#include <QSizeF>

class QPainter;

namespace lumen::plot {

class PlotScene;

/// Renders a PlotScene to a QPainter.
///
/// All colors and sizes come from DesignTokens — no literal values.
/// Rendering order: background, grid, axes, tick labels, axis labels,
/// clipped series, title, legend.
class PlotRenderer {
public:
    /// Render the complete plot.
    void render(QPainter& painter, const PlotScene& scene, QSizeF widgetSize);
};

}  // namespace lumen::plot
