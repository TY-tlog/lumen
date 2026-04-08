#pragma once

#include <QPointF>
#include <QWidget>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::ui {

/// Widget that renders a PlotScene and handles mouse interaction.
///
/// Supports pan (left drag), zoom (scroll wheel, shift/ctrl for
/// axis-specific), zoom box (right drag), reset (double-click),
/// and crosshair with data coordinate tooltip.
class PlotCanvas : public QWidget {
    Q_OBJECT

public:
    explicit PlotCanvas(QWidget* parent = nullptr);

    void setPlotScene(plot::PlotScene* scene);
    [[nodiscard]] plot::PlotScene* plotScene() const { return scene_; }

    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    void drawCrosshair(QPainter& painter);

    plot::PlotScene* scene_ = nullptr;

    // Interaction state.
    bool panning_ = false;
    QPointF panStart_;
    double panStartXMin_ = 0.0;
    double panStartXMax_ = 0.0;
    double panStartYMin_ = 0.0;
    double panStartYMax_ = 0.0;

    bool zoomBoxing_ = false;
    QPoint zoomBoxStart_;
    QPoint zoomBoxEnd_;

    QPointF lastMousePos_;
    bool mouseInWidget_ = false;
};

}  // namespace lumen::ui
