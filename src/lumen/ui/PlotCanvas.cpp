#include "PlotCanvas.h"

#include "InteractionController.h"

#include <plot/CoordinateMapper.h>
#include <plot/HitTester.h>
#include <plot/PlotRenderer.h>
#include <plot/PlotScene.h>
#include <plot/ViewTransform.h>
#include <style/DesignTokens.h>

#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
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

    // Snap to nearest data sample; hide entirely if too far from data.
    auto hit = plot::HitTester::hitTestPoint(*scene_, mapper, mousePos, 20.0);
    if (!hit.has_value()) {
        return;
    }

    // Compute the snapped pixel position from the actual data point.
    QPointF snappedPixel = mapper.dataToPixel(hit->dataPoint.x(), hit->dataPoint.y());

    // Crosshair lines at the snapped position.
    QPen crossPen(lumen::tokens::color::text::tertiary, 1, Qt::DotLine);
    painter.setPen(crossPen);
    painter.drawLine(QPointF(snappedPixel.x(), plotArea.top()),
                     QPointF(snappedPixel.x(), plotArea.bottom()));
    painter.drawLine(QPointF(plotArea.left(), snappedPixel.y()),
                     QPointF(plotArea.right(), snappedPixel.y()));

    // Small marker circle at the snapped point in the series color.
    const auto& allSeries = scene_->series();
    QColor seriesColor = lumen::tokens::color::accent::primary;
    QString seriesName;
    if (hit->seriesIndex >= 0
        && static_cast<std::size_t>(hit->seriesIndex) < allSeries.size()) {
        seriesColor = allSeries[static_cast<std::size_t>(hit->seriesIndex)].style().color;
        seriesName = allSeries[static_cast<std::size_t>(hit->seriesIndex)].name();
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(seriesColor);
    painter.drawEllipse(snappedPixel, 4.0, 4.0);

    // Tooltip with series name and actual data values (6 significant digits).
    QString coords;
    if (seriesName.isEmpty()) {
        coords = QStringLiteral("(%1, %2)")
                     .arg(hit->dataPoint.x(), 0, 'g', 6)
                     .arg(hit->dataPoint.y(), 0, 'g', 6);
    } else {
        coords = QStringLiteral("%1: (%2, %3)")
                     .arg(seriesName)
                     .arg(hit->dataPoint.x(), 0, 'g', 6)
                     .arg(hit->dataPoint.y(), 0, 'g', 6);
    }

    QFont f;
    f.setPixelSize(lumen::tokens::typography::footnote.sizePx);
    painter.setFont(f);
    QFontMetrics fm(f);

    int textW = fm.horizontalAdvance(coords) + 8;
    int textH = fm.height() + 4;

    // Position tooltip to the right and above the snapped point, stay in plot area.
    double tipX = snappedPixel.x() + 10;
    double tipY = snappedPixel.y() - textH - 4;
    if (tipX + textW > plotArea.right()) {
        tipX = snappedPixel.x() - textW - 10;
    }
    if (tipY < plotArea.top()) {
        tipY = snappedPixel.y() + 10;
    }

    QRectF tipRect(tipX, tipY, textW, textH);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 220));
    painter.drawRoundedRect(tipRect, 3, 3);

    painter.setPen(lumen::tokens::color::text::primary);
    painter.drawText(tipRect, Qt::AlignCenter, coords);
}

void PlotCanvas::startTitleEdit() {
    if (scene_ == nullptr) {
        return;
    }

    // If already editing, finish first.
    if (titleEditor_ != nullptr) {
        finishTitleEdit(false);
    }

    // Compute title region: centered at top of widget, using scene margins.
    QFont titleFont;
    titleFont.setPixelSize(scene_->titleFontPx());
    titleFont.setWeight(scene_->titleWeight());
    QFontMetrics fm(titleFont);

    QRectF plotArea = scene_->computePlotArea(size());
    int titleW = qMin(fm.horizontalAdvance(scene_->title()) + 40,
                       static_cast<int>(plotArea.width()));
    int titleH = fm.height() + 8;
    int titleX = static_cast<int>(plotArea.left() + (plotArea.width() - titleW) / 2.0);
    int titleY = 4;  // Small top margin.

    titleEditor_ = new QLineEdit(this);
    titleEditor_->setFont(titleFont);
    titleEditor_->setText(scene_->title());
    titleEditor_->setAlignment(Qt::AlignCenter);
    titleEditor_->setGeometry(titleX, titleY, titleW, titleH);
    titleEditor_->selectAll();
    titleEditor_->setFocus();
    titleEditor_->show();

    // Install event filter for Escape key.
    titleEditor_->installEventFilter(this);

    // Connect Enter/Return to apply.
    connect(titleEditor_, &QLineEdit::returnPressed, this, [this]() {
        finishTitleEdit(true);
    });

    // Enter inline editing mode to suppress pan/zoom.
    controller_->setMode(InteractionMode::EditingTitleInline);
}

void PlotCanvas::finishTitleEdit(bool apply) {
    if (titleEditor_ == nullptr) {
        return;
    }

    if (apply) {
        emit titleEditFinished(titleEditor_->text());
    }

    titleEditor_->removeEventFilter(this);
    titleEditor_->deleteLater();
    titleEditor_ = nullptr;

    // Restore idle mode.
    controller_->setMode(InteractionMode::Idle);
    setFocus();
    update();
}

bool PlotCanvas::eventFilter(QObject* obj, QEvent* event) {
    if (obj == titleEditor_ && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            finishTitleEdit(false);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
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
