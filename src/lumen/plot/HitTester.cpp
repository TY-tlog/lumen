#include "plot/HitTester.h"

#include "plot/CoordinateMapper.h"
#include "plot/LineSeries.h"
#include "plot/PlotScene.h"
#include "style/DesignTokens.h"

#include <QFont>
#include <QFontMetrics>
#include <QRectF>

#include <cmath>
#include <limits>

namespace lumen::plot {

std::optional<HitTester::HitResult> HitTester::hitTest(
    const PlotScene& scene,
    const CoordinateMapper& mapper,
    QPointF pixelPos,
    double tolerancePx)
{
    int bestIndex = -1;
    double bestDistance = std::numeric_limits<double>::max();

    const auto& allSeries = scene.series();
    for (std::size_t i = 0; i < allSeries.size(); ++i) {
        const auto& series = allSeries[i];

        auto polylines = series.buildPolylines();
        double minDistForSeries = std::numeric_limits<double>::max();

        for (const auto& polyline : polylines) {
            for (int j = 0; j + 1 < polyline.size(); ++j) {
                const QPointF& dataA = polyline[j];
                const QPointF& dataB = polyline[j + 1];

                QPointF pixelA = mapper.dataToPixel(dataA.x(), dataA.y());
                QPointF pixelB = mapper.dataToPixel(dataB.x(), dataB.y());

                double dist = pointToSegmentDistance(pixelPos, pixelA, pixelB);
                if (dist < minDistForSeries) {
                    minDistForSeries = dist;
                }
            }
        }

        if (minDistForSeries < bestDistance) {
            bestDistance = minDistForSeries;
            bestIndex = static_cast<int>(i);
        }
    }

    if (bestIndex >= 0 && bestDistance <= tolerancePx) {
        return HitResult{bestIndex, bestDistance};
    }

    return std::nullopt;
}

double HitTester::pointToSegmentDistance(QPointF point, QPointF segA, QPointF segB)
{
    // Vector from A to B.
    const double abx = segB.x() - segA.x();
    const double aby = segB.y() - segA.y();

    // Vector from A to point.
    const double apx = point.x() - segA.x();
    const double apy = point.y() - segA.y();

    const double abLenSq = abx * abx + aby * aby;

    if (abLenSq < 1e-12) {
        // Degenerate segment (A == B): distance is just point-to-A.
        return std::sqrt(apx * apx + apy * apy);
    }

    // Project point onto the line defined by A-B, clamped to [0, 1].
    double t = (apx * abx + apy * aby) / abLenSq;
    t = std::clamp(t, 0.0, 1.0);

    // Closest point on segment.
    const double closestX = segA.x() + t * abx;
    const double closestY = segA.y() + t * aby;

    const double dx = point.x() - closestX;
    const double dy = point.y() - closestY;

    return std::sqrt(dx * dx + dy * dy);
}

QRectF HitTester::computeLegendRect(const PlotScene& scene, QRectF plotArea)
{
    // Mirror the legend layout logic in PlotRenderer.
    constexpr int kLegendPadding = 8;
    constexpr int kLegendLineLength = 20;
    constexpr int kLegendSpacing = 4;

    if (scene.seriesCount() <= 1) {
        return {};  // No legend drawn for 0-1 series.
    }

    QFont bodyFont;
    bodyFont.setPixelSize(tokens::typography::body.sizePx);
    bodyFont.setWeight(tokens::typography::body.weight);
    QFontMetrics fm(bodyFont);

    int legendHeight = static_cast<int>(scene.seriesCount()) *
                       (fm.height() + kLegendSpacing) + kLegendPadding;
    int legendWidth = 0;
    for (const auto& s : scene.series()) {
        int textWidth = fm.horizontalAdvance(s.name().isEmpty()
                                             ? QStringLiteral("Series")
                                             : s.name());
        legendWidth = std::max(legendWidth, textWidth);
    }
    legendWidth += kLegendLineLength + kLegendSpacing * 2 + kLegendPadding * 2;

    return QRectF(plotArea.right() - legendWidth - kLegendPadding,
                  plotArea.top() + kLegendPadding,
                  legendWidth, legendHeight);
}

RegionHitResult HitTester::hitNonSeriesElement(
    const PlotScene& scene,
    QSizeF widgetSize,
    QPointF pixelPos)
{
    QRectF plotArea = scene.computePlotArea(widgetSize);

    // 1. Title region: above the plot area, spanning its width.
    if (!scene.title().isEmpty()) {
        QRectF titleRect(plotArea.left(), 0.0,
                         plotArea.width(), plotArea.top());
        if (titleRect.contains(pixelPos)) {
            return {HitKind::Title};
        }
    }

    // 2. Legend region (top-right inside plot area).
    {
        QRectF legendRect = computeLegendRect(scene, plotArea);
        if (legendRect.isValid() && legendRect.contains(pixelPos)) {
            return {HitKind::Legend};
        }
    }

    // 3. X axis band: below the plot area.
    {
        QRectF xAxisBand(plotArea.left(), plotArea.bottom(),
                         plotArea.width(), widgetSize.height() - plotArea.bottom());
        if (xAxisBand.contains(pixelPos)) {
            return {HitKind::XAxis};
        }
    }

    // 4. Y axis band: left of the plot area.
    {
        QRectF yAxisBand(0.0, plotArea.top(),
                         plotArea.left(), plotArea.height());
        if (yAxisBand.contains(pixelPos)) {
            return {HitKind::YAxis};
        }
    }

    // 5. Inside the plot area.
    if (plotArea.contains(pixelPos)) {
        return {HitKind::PlotArea};
    }

    return {HitKind::None};
}

}  // namespace lumen::plot
