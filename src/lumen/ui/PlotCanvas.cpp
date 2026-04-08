#include "PlotCanvas.h"

#include <plot/CoordinateMapper.h>
#include <plot/PlotRenderer.h>
#include <plot/PlotScene.h>
#include <plot/ViewTransform.h>
#include <style/DesignTokens.h>

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

namespace lumen::ui {

namespace {
constexpr double kZoomInFactor = 1.2;
constexpr double kZoomOutFactor = 1.0 / 1.2;
}  // namespace

PlotCanvas::PlotCanvas(QWidget* parent)
    : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
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
    if (zoomBoxing_) {
        QPen boxPen(lumen::tokens::color::accent::primary, 1, Qt::DashLine);
        painter.setPen(boxPen);
        painter.setBrush(QColor(10, 132, 255, 30));
        painter.drawRect(QRect(zoomBoxStart_, zoomBoxEnd_).normalized());
    }

    // Draw crosshair if mouse is in widget and not dragging.
    if (mouseInWidget_ && !panning_ && !zoomBoxing_) {
        drawCrosshair(painter);
    }
}

void PlotCanvas::drawCrosshair(QPainter& painter) {
    if (scene_ == nullptr) {
        return;
    }

    QRectF plotArea = scene_->computePlotArea(size());
    if (!plotArea.contains(lastMousePos_)) {
        return;
    }

    const auto& vt = scene_->viewTransform();
    plot::CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);
    auto [dataX, dataY] = mapper.pixelToData(lastMousePos_);

    // Crosshair lines.
    QPen crossPen(lumen::tokens::color::text::tertiary, 1, Qt::DotLine);
    painter.setPen(crossPen);
    painter.drawLine(QPointF(lastMousePos_.x(), plotArea.top()),
                     QPointF(lastMousePos_.x(), plotArea.bottom()));
    painter.drawLine(QPointF(plotArea.left(), lastMousePos_.y()),
                     QPointF(plotArea.right(), lastMousePos_.y()));

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
    double tipX = lastMousePos_.x() + 10;
    double tipY = lastMousePos_.y() - textH - 4;
    if (tipX + textW > plotArea.right()) {
        tipX = lastMousePos_.x() - textW - 10;
    }
    if (tipY < plotArea.top()) {
        tipY = lastMousePos_.y() + 10;
    }

    QRectF tipRect(tipX, tipY, textW, textH);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 220));
    painter.drawRoundedRect(tipRect, 3, 3);

    painter.setPen(lumen::tokens::color::text::primary);
    painter.drawText(tipRect, Qt::AlignCenter, coords);
}

void PlotCanvas::mousePressEvent(QMouseEvent* event) {
    if (scene_ == nullptr) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        panning_ = true;
        panStart_ = event->position();
        const auto& vt = scene_->viewTransform();
        panStartXMin_ = vt.xMin();
        panStartXMax_ = vt.xMax();
        panStartYMin_ = vt.yMin();
        panStartYMax_ = vt.yMax();
        setCursor(Qt::ClosedHandCursor);
    } else if (event->button() == Qt::RightButton) {
        zoomBoxing_ = true;
        zoomBoxStart_ = event->pos();
        zoomBoxEnd_ = event->pos();
    }
}

void PlotCanvas::mouseMoveEvent(QMouseEvent* event) {
    lastMousePos_ = event->position();
    mouseInWidget_ = true;

    if (panning_ && scene_ != nullptr) {
        QRectF plotArea = scene_->computePlotArea(size());
        const auto& vt = scene_->viewTransform();
        plot::CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);

        auto [startDataX, startDataY] = mapper.pixelToData(panStart_);
        auto [curDataX, curDataY] = mapper.pixelToData(lastMousePos_);

        double dx = startDataX - curDataX;
        double dy = startDataY - curDataY;

        // Reset to start and apply new pan.
        scene_->viewTransform().setBaseRange(panStartXMin_, panStartXMax_,
                                              panStartYMin_, panStartYMax_);
        scene_->viewTransform().reset();
        scene_->viewTransform().pan(dx, dy);
        update();
        return;
    }

    if (zoomBoxing_) {
        zoomBoxEnd_ = event->pos();
        update();
        return;
    }

    // Crosshair update.
    update();
}

void PlotCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && panning_) {
        panning_ = false;
        setCursor(Qt::ArrowCursor);
        // Update base range to current view so future pans work correctly.
        auto& vt = scene_->viewTransform();
        vt.setBaseRange(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax());
    }

    if (event->button() == Qt::RightButton && zoomBoxing_ && scene_ != nullptr) {
        zoomBoxing_ = false;
        QRect box = QRect(zoomBoxStart_, zoomBoxEnd_).normalized();
        if (box.width() > 5 && box.height() > 5) {
            QRectF plotArea = scene_->computePlotArea(size());
            const auto& vt = scene_->viewTransform();
            plot::CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);

            auto [x1, y1] = mapper.pixelToData(box.topLeft());
            auto [x2, y2] = mapper.pixelToData(box.bottomRight());

            double xMin = std::min(x1, x2);
            double xMax = std::max(x1, x2);
            double yMin = std::min(y1, y2);
            double yMax = std::max(y1, y2);

            scene_->viewTransform().setBaseRange(xMin, xMax, yMin, yMax);
            scene_->viewTransform().reset();
        }
        update();
    }
}

void PlotCanvas::wheelEvent(QWheelEvent* event) {
    if (scene_ == nullptr) {
        return;
    }

    double factor = (event->angleDelta().y() > 0) ? kZoomInFactor : kZoomOutFactor;

    QRectF plotArea = scene_->computePlotArea(size());
    const auto& vt = scene_->viewTransform();
    plot::CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);

    auto [cx, cy] = mapper.pixelToData(event->position());

    if (event->modifiers() & Qt::ShiftModifier) {
        scene_->viewTransform().zoomX(factor, cx);
    } else if (event->modifiers() & Qt::ControlModifier) {
        scene_->viewTransform().zoomY(factor, cy);
    } else {
        scene_->viewTransform().zoom(factor, cx, cy);
    }

    // Update base range so panning works from the new zoom level.
    auto& vtMut = scene_->viewTransform();
    vtMut.setBaseRange(vtMut.xMin(), vtMut.xMax(), vtMut.yMin(), vtMut.yMax());

    update();
    event->accept();
}

void PlotCanvas::mouseDoubleClickEvent(QMouseEvent* /*event*/) {
    if (scene_ == nullptr) {
        return;
    }
    scene_->autoRange();
    update();
}

}  // namespace lumen::ui
