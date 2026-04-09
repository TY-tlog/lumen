#include "InteractionController.h"

#include "PlotCanvas.h"

#include <plot/CoordinateMapper.h>
#include <plot/HitTester.h>
#include <plot/PlotScene.h>
#include <plot/ViewTransform.h>

#include <QMouseEvent>
#include <QWheelEvent>

#include <algorithm>

namespace lumen::ui {

namespace {
constexpr double kZoomInFactor = 1.2;
constexpr double kZoomOutFactor = 1.0 / 1.2;
}  // namespace

InteractionController::InteractionController(PlotCanvas* canvas, QObject* parent)
    : QObject(parent)
    , canvas_(canvas)
{
}

void InteractionController::handleMousePress(QMouseEvent* event)
{
    auto* scene = canvas_->plotScene();
    if (scene == nullptr) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        mode_ = InteractionMode::Panning;
        panStart_ = event->position();
        const auto& vt = scene->viewTransform();
        panStartXMin_ = vt.xMin();
        panStartXMax_ = vt.xMax();
        panStartYMin_ = vt.yMin();
        panStartYMax_ = vt.yMax();
        canvas_->setCursor(Qt::ClosedHandCursor);
    } else if (event->button() == Qt::RightButton) {
        mode_ = InteractionMode::ZoomBoxing;
        zoomBoxStart_ = event->pos();
        zoomBoxEnd_ = event->pos();
    }
}

void InteractionController::handleMouseMove(QMouseEvent* event)
{
    lastMousePos_ = event->position();
    mouseInWidget_ = true;

    auto* scene = canvas_->plotScene();

    if (mode_ == InteractionMode::Panning && scene != nullptr) {
        QRectF plotArea = scene->computePlotArea(canvas_->size());
        const auto& vt = scene->viewTransform();
        plot::CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);

        auto [startDataX, startDataY] = mapper.pixelToData(panStart_);
        auto [curDataX, curDataY] = mapper.pixelToData(lastMousePos_);

        double dx = startDataX - curDataX;
        double dy = startDataY - curDataY;

        // Reset to start and apply new pan.
        scene->viewTransform().setBaseRange(panStartXMin_, panStartXMax_,
                                            panStartYMin_, panStartYMax_);
        scene->viewTransform().reset();
        scene->viewTransform().pan(dx, dy);
        emit requestRepaint();
        return;
    }

    if (mode_ == InteractionMode::ZoomBoxing) {
        zoomBoxEnd_ = event->pos();
        emit requestRepaint();
        return;
    }

    // Crosshair update.
    emit requestRepaint();
}

void InteractionController::handleMouseRelease(QMouseEvent* event)
{
    auto* scene = canvas_->plotScene();

    if (event->button() == Qt::LeftButton && mode_ == InteractionMode::Panning) {
        mode_ = InteractionMode::Idle;
        canvas_->setCursor(Qt::ArrowCursor);
        // Update base range to current view so future pans work correctly.
        if (scene != nullptr) {
            auto& vt = scene->viewTransform();
            vt.setBaseRange(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax());
        }
    }

    if (event->button() == Qt::RightButton && mode_ == InteractionMode::ZoomBoxing
        && scene != nullptr) {
        mode_ = InteractionMode::Idle;
        QRect box = QRect(zoomBoxStart_, zoomBoxEnd_).normalized();
        if (box.width() > 5 && box.height() > 5) {
            QRectF plotArea = scene->computePlotArea(canvas_->size());
            const auto& vt = scene->viewTransform();
            plot::CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);

            auto [x1, y1] = mapper.pixelToData(box.topLeft());
            auto [x2, y2] = mapper.pixelToData(box.bottomRight());

            double xMin = std::min(x1, x2);
            double xMax = std::max(x1, x2);
            double yMin = std::min(y1, y2);
            double yMax = std::max(y1, y2);

            scene->viewTransform().setBaseRange(xMin, xMax, yMin, yMax);
            scene->viewTransform().reset();
        }
        emit requestRepaint();
    }
}

void InteractionController::handleWheel(QWheelEvent* event)
{
    auto* scene = canvas_->plotScene();
    if (scene == nullptr) {
        return;
    }

    double factor = (event->angleDelta().y() > 0) ? kZoomInFactor : kZoomOutFactor;

    QRectF plotArea = scene->computePlotArea(canvas_->size());
    const auto& vt = scene->viewTransform();
    plot::CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);

    auto [cx, cy] = mapper.pixelToData(event->position());

    if (event->modifiers() & Qt::ShiftModifier) {
        scene->viewTransform().zoomX(factor, cx);
    } else if (event->modifiers() & Qt::ControlModifier) {
        scene->viewTransform().zoomY(factor, cy);
    } else {
        scene->viewTransform().zoom(factor, cx, cy);
    }

    // Update base range so panning works from the new zoom level.
    auto& vtMut = scene->viewTransform();
    vtMut.setBaseRange(vtMut.xMin(), vtMut.xMax(), vtMut.yMin(), vtMut.yMax());

    emit requestRepaint();
    event->accept();
}

void InteractionController::handleDoubleClick(QMouseEvent* event)
{
    auto* scene = canvas_->plotScene();
    if (scene == nullptr) {
        return;
    }

    QRectF plotArea = scene->computePlotArea(canvas_->size());
    const auto& vt = scene->viewTransform();
    plot::CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);

    auto result = plot::HitTester::hitTest(*scene, mapper, event->position());
    if (result.has_value()) {
        emit seriesDoubleClicked(result->seriesIndex);
    } else {
        emit emptyAreaDoubleClicked();
    }
}

bool InteractionController::isMouseInPlotArea() const
{
    if (!mouseInWidget_ || canvas_->plotScene() == nullptr) {
        return false;
    }
    QRectF plotArea = canvas_->plotScene()->computePlotArea(canvas_->size());
    return plotArea.contains(lastMousePos_);
}

QRect InteractionController::zoomBoxRect() const
{
    return QRect(zoomBoxStart_, zoomBoxEnd_).normalized();
}

}  // namespace lumen::ui
