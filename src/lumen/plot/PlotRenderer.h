#pragma once

#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>

class QPainter;

namespace lumen::plot {

class PlotScene;

/// Renders a PlotScene to a QPainter.
///
/// All colors and sizes come from DesignTokens — no literal values.
/// Rendering order: background, grid, axes, tick labels, axis labels,
/// clipped series, annotations, title, legend.
class PlotRenderer {
public:
    /// Render the complete plot.
    void render(QPainter& painter, const PlotScene& scene, QSizeF widgetSize);

    /// Enable text-as-path mode for SVG/PDF export (ADR-055).
    /// When true, all text is rendered via QPainterPath outlines
    /// instead of drawText, ensuring cross-viewer consistency.
    void setTextAsPath(bool enabled) { textAsPath_ = enabled; }
    [[nodiscard]] bool textAsPath() const { return textAsPath_; }

private:
    /// Draw text — either drawText or drawPath depending on mode.
    void drawTextOrPath(QPainter& painter, const QRectF& rect,
                        int flags, const QString& text) const;
    void drawTextOrPath(QPainter& painter, const QPointF& pos,
                        const QString& text) const;

    bool textAsPath_ = false;
};

}  // namespace lumen::plot
