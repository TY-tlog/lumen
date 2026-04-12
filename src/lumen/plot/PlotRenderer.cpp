#include "plot/PlotRenderer.h"

#include "plot/Axis.h"
#include "plot/CoordinateMapper.h"
#include "plot/Legend.h"
#include "plot/LineSeries.h"
#include "plot/PlotItem.h"
#include "plot/PlotScene.h"
#include "style/DesignTokens.h"

#include <QPainter>
#include <QPen>
#include <QRectF>

namespace lumen::plot {

namespace {

constexpr int kTickLength = 4;
constexpr int kTickLabelPadding = 4;
constexpr int kLegendPadding = 8;
constexpr int kLegendLineLength = 20;
constexpr int kLegendSpacing = 4;

QFont footnoteFont() {
    QFont f;
    f.setPixelSize(tokens::typography::footnote.sizePx);
    f.setWeight(tokens::typography::footnote.weight);
    return f;
}

QFont bodyStrongFont() {
    QFont f;
    f.setPixelSize(tokens::typography::bodyStrong.sizePx);
    f.setWeight(tokens::typography::bodyStrong.weight);
    return f;
}

QFont titleFontFromScene(const PlotScene& scene) {
    QFont f;
    f.setPixelSize(scene.titleFontPx());
    f.setWeight(scene.titleWeight());
    return f;
}

QFont bodyFont() {
    QFont f;
    f.setPixelSize(tokens::typography::body.sizePx);
    f.setWeight(tokens::typography::body.weight);
    return f;
}

/// Compute the legend bounding rect for a given position.
QRectF computeLegendRect(LegendPosition position, const QRectF& plotArea,
                         int legendWidth, int legendHeight) {
    switch (position) {
        case LegendPosition::TopLeft:
            return QRectF(plotArea.left() + kLegendPadding,
                          plotArea.top() + kLegendPadding,
                          legendWidth, legendHeight);
        case LegendPosition::TopRight:
            return QRectF(plotArea.right() - legendWidth - kLegendPadding,
                          plotArea.top() + kLegendPadding,
                          legendWidth, legendHeight);
        case LegendPosition::BottomLeft:
            return QRectF(plotArea.left() + kLegendPadding,
                          plotArea.bottom() - legendHeight - kLegendPadding,
                          legendWidth, legendHeight);
        case LegendPosition::BottomRight:
            return QRectF(plotArea.right() - legendWidth - kLegendPadding,
                          plotArea.bottom() - legendHeight - kLegendPadding,
                          legendWidth, legendHeight);
        case LegendPosition::OutsideRight:
            return QRectF(plotArea.right() + kLegendPadding,
                          plotArea.top() + kLegendPadding,
                          legendWidth, legendHeight);
    }
    // Fallback to TopRight.
    return QRectF(plotArea.right() - legendWidth - kLegendPadding,
                  plotArea.top() + kLegendPadding,
                  legendWidth, legendHeight);
}

}  // namespace

void PlotRenderer::render(QPainter& painter, const PlotScene& scene, QSizeF widgetSize) {
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // 1. Background.
    painter.fillRect(QRectF({0, 0}, widgetSize), tokens::color::background::primary);

    // 2. Compute layout.
    QRectF plotArea = scene.computePlotArea(widgetSize);

    // 3. Create coordinate mapper from current view transform.
    const auto& vt = scene.viewTransform();
    CoordinateMapper mapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);

    // 4. Get ticks.
    auto xTicks = scene.xAxis().ticks();
    auto yTicks = scene.yAxis().ticks();

    // 5. Draw grid lines (border.subtle, dashed) — respect gridVisible per axis.
    {
        QPen gridPen(tokens::color::border::subtle, tokens::plot::gridLineWidth, Qt::DashLine);
        painter.setPen(gridPen);

        if (scene.xAxis().gridVisible()) {
            for (const auto& tick : xTicks) {
                auto px = mapper.dataToPixel(tick.value, vt.yMin());
                if (px.x() >= plotArea.left() && px.x() <= plotArea.right()) {
                    painter.drawLine(QPointF(px.x(), plotArea.top()),
                                     QPointF(px.x(), plotArea.bottom()));
                }
            }
        }
        if (scene.yAxis().gridVisible()) {
            for (const auto& tick : yTicks) {
                auto px = mapper.dataToPixel(vt.xMin(), tick.value);
                if (px.y() >= plotArea.top() && px.y() <= plotArea.bottom()) {
                    painter.drawLine(QPointF(plotArea.left(), px.y()),
                                     QPointF(plotArea.right(), px.y()));
                }
            }
        }
    }

    // 6. Draw axis lines (border.strong).
    {
        QPen axisPen(tokens::color::border::strong, tokens::plot::axisLineWidth);
        painter.setPen(axisPen);
        // Left edge.
        painter.drawLine(QPointF(plotArea.left(), plotArea.top()),
                         QPointF(plotArea.left(), plotArea.bottom()));
        // Bottom edge.
        painter.drawLine(QPointF(plotArea.left(), plotArea.bottom()),
                         QPointF(plotArea.right(), plotArea.bottom()));
    }

    // 7. Draw tick marks and tick labels.
    {
        QPen tickPen(tokens::color::border::strong, 1);
        painter.setPen(tickPen);
        painter.setFont(footnoteFont());

        // X axis ticks.
        for (const auto& tick : xTicks) {
            auto px = mapper.dataToPixel(tick.value, vt.yMin());
            if (px.x() < plotArea.left() || px.x() > plotArea.right()) {
                continue;
            }
            // Tick mark.
            painter.drawLine(QPointF(px.x(), plotArea.bottom()),
                             QPointF(px.x(), plotArea.bottom() + kTickLength));
            // Tick label.
            painter.setPen(tokens::color::text::secondary);
            QRectF labelRect(px.x() - 40, plotArea.bottom() + kTickLength + kTickLabelPadding,
                             80, 20);
            painter.drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, tick.label);
            painter.setPen(tickPen);
        }

        // Y axis ticks.
        for (const auto& tick : yTicks) {
            auto px = mapper.dataToPixel(vt.xMin(), tick.value);
            if (px.y() < plotArea.top() || px.y() > plotArea.bottom()) {
                continue;
            }
            // Tick mark.
            painter.drawLine(QPointF(plotArea.left() - kTickLength, px.y()),
                             QPointF(plotArea.left(), px.y()));
            // Tick label.
            painter.setPen(tokens::color::text::secondary);
            QRectF labelRect(plotArea.left() - kTickLength - kTickLabelPadding - 45, px.y() - 10,
                             45, 20);
            painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, tick.label);
            painter.setPen(tickPen);
        }
    }

    // 8. Draw axis labels.
    {
        painter.setPen(tokens::color::text::primary);
        painter.setFont(bodyStrongFont());

        // X axis label — centered below ticks.
        if (!scene.xAxis().label().isEmpty()) {
            QRectF xLabelRect(plotArea.left(), plotArea.bottom() + 30,
                              plotArea.width(), 20);
            painter.drawText(xLabelRect, Qt::AlignHCenter | Qt::AlignTop,
                             scene.xAxis().label());
        }

        // Y axis label — rotated, centered left of ticks.
        if (!scene.yAxis().label().isEmpty()) {
            painter.save();
            painter.translate(15, plotArea.center().y());
            painter.rotate(-90);
            QRectF yLabelRect(-plotArea.height() / 2, -10, plotArea.height(), 20);
            painter.drawText(yLabelRect, Qt::AlignHCenter | Qt::AlignVCenter,
                             scene.yAxis().label());
            painter.restore();
        }
    }

    // 9. Clip to plot area and draw series/items.
    {
        painter.save();
        painter.setClipRect(plotArea);

        for (const auto& item : scene.items()) {
            if (!item->isVisible()) {
                continue;
            }
            item->paint(&painter, mapper, plotArea);
        }

        painter.restore();
    }

    // 10. Title — use PlotScene font properties instead of hardcoded title3.
    if (!scene.title().isEmpty()) {
        painter.setPen(tokens::color::text::primary);
        painter.setFont(titleFontFromScene(scene));
        QRectF titleRect(plotArea.left(), 4, plotArea.width(), 24);
        painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter,
                         scene.title());
    }

    // 11. Legend — uses Legend state for position and visibility.
    const auto& legendState = scene.legend();
    if (legendState.isVisible() && scene.itemCount() > 1) {
        painter.setFont(bodyFont());
        QFontMetrics fm(painter.font());

        constexpr int kLegendRowHeight = 18;
        int legendHeight = static_cast<int>(scene.itemCount()) *
                           kLegendRowHeight + kLegendPadding;
        int legendWidth = 0;
        for (const auto& item : scene.items()) {
            int textWidth = fm.horizontalAdvance(item->name().isEmpty() ? "Series" : item->name());
            legendWidth = std::max(legendWidth, textWidth);
        }
        legendWidth += kLegendLineLength + kLegendSpacing * 2 + kLegendPadding * 2;

        QRectF legendRect = computeLegendRect(
            legendState.position(), plotArea, legendWidth, legendHeight);

        // Background.
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 220));
        painter.drawRoundedRect(legendRect, tokens::radius::xs, tokens::radius::xs);

        // Entries — T13: mixed swatches by item type.
        constexpr int kRowHeight = 18;
        double y = legendRect.top() + kLegendPadding;
        for (const auto& item : scene.items()) {
            double x = legendRect.left() + kLegendPadding;

            const float opacity = item->isVisible() ? 1.0F : 0.35F;

            QColor itemColor = item->primaryColor();
            itemColor.setAlphaF(opacity);
            double swatchCenterY = y + kRowHeight / 2.0;

            switch (item->type()) {
            case PlotItem::Type::Scatter: {
                // Draw a filled circle marker.
                painter.setPen(Qt::NoPen);
                painter.setBrush(itemColor);
                double r = 4.0;
                double cx = x + kLegendLineLength / 2.0;
                painter.drawEllipse(QPointF(cx, swatchCenterY), r, r);
                break;
            }
            case PlotItem::Type::Bar: {
                // Draw a filled 12x8 rectangle.
                painter.setPen(Qt::NoPen);
                painter.setBrush(itemColor);
                double rectW = 12.0;
                double rectH = 8.0;
                double rx = x + (kLegendLineLength - rectW) / 2.0;
                double ry = swatchCenterY - rectH / 2.0;
                painter.drawRect(QRectF(rx, ry, rectW, rectH));
                break;
            }
            case PlotItem::Type::Line:
            default: {
                // Draw a 20px line segment.
                QPen entryPen(itemColor, 2);
                painter.setPen(entryPen);
                painter.drawLine(QPointF(x, swatchCenterY),
                                 QPointF(x + kLegendLineLength, swatchCenterY));
                break;
            }
            }

            // Name text.
            QColor textColor = tokens::color::text::secondary;
            textColor.setAlphaF(opacity);
            painter.setPen(textColor);
            QString label = item->name().isEmpty() ? QStringLiteral("Series") : item->name();
            painter.drawText(QPointF(x + kLegendLineLength + kLegendSpacing, y + fm.ascent()),
                             label);

            y += kRowHeight;
        }
    }
}

}  // namespace lumen::plot
