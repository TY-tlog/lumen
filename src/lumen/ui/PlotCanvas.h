#pragma once

#include <QWidget>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::ui {

class InteractionController;

/// Widget that renders a PlotScene and delegates interaction to
/// InteractionController.
///
/// PlotCanvas is a thin rendering host: it paints the plot via
/// PlotRenderer and draws overlays (zoom box, crosshair) using
/// state from InteractionController.  All mouse/wheel logic lives
/// in InteractionController (see ADR-020).
class PlotCanvas : public QWidget {
    Q_OBJECT

public:
    explicit PlotCanvas(QWidget* parent = nullptr);

    void setPlotScene(plot::PlotScene* scene);
    [[nodiscard]] plot::PlotScene* plotScene() const { return scene_; }

    [[nodiscard]] InteractionController* controller() const { return controller_; }

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
    InteractionController* controller_ = nullptr;
};

}  // namespace lumen::ui
