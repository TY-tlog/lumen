#pragma once

#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QRect>

class QMouseEvent;
class QWheelEvent;

namespace lumen::ui {

class PlotCanvas;

enum class InteractionMode { Idle, Panning, ZoomBoxing, EditingTitleInline };

/// Handles all mouse/wheel interactions for PlotCanvas.
///
/// Extracted from PlotCanvas (ADR-020) so that PlotCanvas is a thin
/// rendering host.  InteractionController owns all interaction state
/// (pan, zoom-box, crosshair position) and emits signals for events
/// that higher layers need (series double-click, repaint requests).
class InteractionController : public QObject {
    Q_OBJECT

public:
    explicit InteractionController(PlotCanvas* canvas, QObject* parent = nullptr);

    void handleMousePress(QMouseEvent* event);
    void handleMouseMove(QMouseEvent* event);
    void handleMouseRelease(QMouseEvent* event);
    void handleWheel(QWheelEvent* event);
    void handleDoubleClick(QMouseEvent* event);

    [[nodiscard]] InteractionMode mode() const { return mode_; }
    [[nodiscard]] QPointF lastMousePos() const { return lastMousePos_; }
    [[nodiscard]] bool isMouseInPlotArea() const;
    [[nodiscard]] bool isZoomBoxActive() const { return mode_ == InteractionMode::ZoomBoxing; }
    [[nodiscard]] QRect zoomBoxRect() const;

    /// Set the interaction mode externally (e.g. for inline title editing).
    void setMode(InteractionMode m) { mode_ = m; }

signals:
    // Phase 3a signals.
    void seriesDoubleClicked(int seriesIndex);
    void emptyAreaDoubleClicked();
    void requestRepaint();
    // Phase 3b signals.
    void xAxisDoubleClicked();
    void yAxisDoubleClicked();
    void titleDoubleClicked();
    void legendDoubleClicked();

private:
    PlotCanvas* canvas_;
    InteractionMode mode_ = InteractionMode::Idle;

    // Pan state.
    QPointF panStart_;
    double panStartXMin_ = 0.0;
    double panStartXMax_ = 0.0;
    double panStartYMin_ = 0.0;
    double panStartYMax_ = 0.0;

    // Zoom-box state.
    QPoint zoomBoxStart_;
    QPoint zoomBoxEnd_;

    // Crosshair tracking.
    QPointF lastMousePos_;
    bool mouseInWidget_ = false;
};

}  // namespace lumen::ui
