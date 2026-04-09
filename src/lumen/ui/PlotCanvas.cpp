#include "PlotCanvas.h"

#include "InteractionController.h"

#include <plot/CoordinateMapper.h>
#include <plot/PlotRenderer.h>
#include <plot/PlotScene.h>
#include <plot/ViewTransform.h>
#include <style/DesignTokens.h>

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

namespace lumen::ui {

PlotCanvas::PlotCanvas(QWidget* parent)
    : QWidget(parent)
    , controller_(new InteractionController(this, this))
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    connect(controller_, &InteractionController::requestRepaint, this,
            QOverload<>::of(&QWidget::update));
    connect(controller_, &InteractionController::emptyAreaDoubleClicked, this, [this]() {
        if (scene_ != nullptr) {
            scene_->autoRange();
            update();
        }
    });
}

void PlotCanvas::setPlotScene(plot::PlotScene* scene) {
    scene_ = scene;
    update();
}

QSize PlotCanvas::minimumSizeHint() const {
    return {200, 150};
}

void PlotCanvas::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);

    if (scene_ == nullptr) {
        painter.fillRect(rect(), lumen::tokens::color::background::primary);
        painter.setPen(lumen::tokens::color::text::tertiary);
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("No plot data"));
        return;
    }

    plot::PlotRenderer renderer;
    renderer.render(painter, *scene_, size());

    // Draw zoom box if active.
    if (controller_->isZoomBoxActive()) {
        QPen boxPen(lumen::tokens::color::accent::primary, 1, Qt::DashLine);
        painter.setPen(boxPen);
        painter.setBrush(QColor(10, 132, 255, 30));
        painter.drawRect(controller_->zoomBoxRect());
    }

    // Draw crosshair if mouse is in plot area and not dragging.
    if (controller_->isMouseInPlotArea()
        && controller_->mode() == InteractionMode::Idle) {
        drawCrosshair(painter);
    }
}

void PlotCanvas::drawCrosshair(QPainter& painter) {
    if (scene_ == nullptr) {
        return;
    }

    QRectF plotArea = scene_->computePlotArea(size());
    QPointF mousePos = controller_->lastMousePos();
    if (!plotArea.contains(mousePos)) {
        return;
    }

    const auto& vt = scene_->viewTransform();
    plot::CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);
    auto [dataX, dataY] = mapper.pixelToData(mousePos);

    // Crosshair lines.
    QPen crossPen(lumen::tokens::color::text::tertiary, 1, Qt::DotLine);
    painter.setPen(crossPen);
    painter.drawLine(QPointF(mousePos.x(), plotArea.top()),
                     QPointF(mousePos.x(), plotArea.bottom()));
    painter.drawLine(QPointF(plotArea.left(), mousePos.y()),
                     QPointF(plotArea.right(), mousePos.y()));

    // Coordinate tooltip.
    QString coords = QStringLiteral("(%1, %2)")
                         .arg(dataX, 0, 'g', 4)
                         .arg(dataY, 0, 'g', 4);

    QFont f;
    f.setPixelSize(lumen::tokens::typography::footnote.sizePx);
    painter.setFont(f);
    QFontMetrics fm(f);

    int textW = fm.horizontalAdvance(coords) + 8;
    int textH = fm.height() + 4;

    // Position tooltip to the right and above cursor, stay in plot area.
    double tipX = mousePos.x() + 10;
    double tipY = mousePos.y() - textH - 4;
    if (tipX + textW > plotArea.right()) {
        tipX = mousePos.x() - textW - 10;
    }
    if (tipY < plotArea.top()) {
        tipY = mousePos.y() + 10;
    }

    QRectF tipRect(tipX, tipY, textW, textH);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 220));
    painter.drawRoundedRect(tipRect, 3, 3);

    painter.setPen(lumen::tokens::color::text::primary);
    painter.drawText(tipRect, Qt::AlignCenter, coords);
}

void PlotCanvas::mousePressEvent(QMouseEvent* event) {
    controller_->handleMousePress(event);
}

void PlotCanvas::mouseMoveEvent(QMouseEvent* event) {
    controller_->handleMouseMove(event);
}

void PlotCanvas::mouseReleaseEvent(QMouseEvent* event) {
    controller_->handleMouseRelease(event);
}

void PlotCanvas::wheelEvent(QWheelEvent* event) {
    controller_->handleWheel(event);
}

void PlotCanvas::mouseDoubleClickEvent(QMouseEvent* event) {
    controller_->handleDoubleClick(event);
}

}  // namespace lumen::ui
