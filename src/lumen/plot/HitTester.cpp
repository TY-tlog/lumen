#include "plot/HitTester.h"

#include "data/Column.h"
#include "plot/CoordinateMapper.h"
#include "plot/LineSeries.h"
#include "plot/PlotScene.h"
#include "style/DesignTokens.h"

#include <QFont>
#include <QFontMetrics>
#include <QRectF>

#include <algorithm>
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

std::optional<PointHitResult> HitTester::hitTestPoint(
    const PlotScene& scene,
    const CoordinateMapper& mapper,
    QPointF pixelPos,
    double maxPixelDistance)
{
    int bestSeriesIndex = -1;
    std::size_t bestSampleIndex = 0;
    QPointF bestDataPoint;
    double bestPixelDist = std::numeric_limits<double>::max();

    auto [cursorDataX, cursorDataY] = mapper.pixelToData(pixelPos);

    const auto& allSeries = scene.series();
    for (std::size_t si = 0; si < allSeries.size(); ++si) {
        const auto& series = allSeries[si];
        if (!series.isVisible()) {
            continue;
        }

        const auto* xCol = series.xColumn();
        const auto* yCol = series.yColumn();
        if (xCol == nullptr || yCol == nullptr) {
            continue;
        }

        const auto& xData = xCol->doubleData();
        const auto& yData = yCol->doubleData();
        const auto count = xData.size();
        if (count == 0) {
            continue;
        }

        // Binary search for approximate position of cursorDataX.
        auto it = std::lower_bound(xData.begin(), xData.end(), cursorDataX);
        auto approxIdx = std::distance(xData.begin(), it);

        // Check a window of +/-5 samples around the binary search position
        // for robustness against non-perfectly-sorted data.
        constexpr std::ptrdiff_t kWindow = 5;
        std::ptrdiff_t lo = std::max(std::ptrdiff_t{0}, approxIdx - kWindow);
        std::ptrdiff_t hi = std::min(static_cast<std::ptrdiff_t>(count),
                                     approxIdx + kWindow + 1);

        for (std::ptrdiff_t j = lo; j < hi; ++j) {
            const double dx = xData[static_cast<std::size_t>(j)];
            const double dy = yData[static_cast<std::size_t>(j)];

            // Skip NaN samples.
            if (std::isnan(dx) || std::isnan(dy)) {
                continue;
            }

            QPointF samplePixel = mapper.dataToPixel(dx, dy);
            double pdx = pixelPos.x() - samplePixel.x();
            double pdy = pixelPos.y() - samplePixel.y();
            double dist = std::sqrt(pdx * pdx + pdy * pdy);

            if (dist < bestPixelDist) {
                bestPixelDist = dist;
                bestSeriesIndex = static_cast<int>(si);
                bestSampleIndex = static_cast<std::size_t>(j);
                bestDataPoint = QPointF(dx, dy);
            }
        }
    }

    if (bestSeriesIndex >= 0 && bestPixelDist <= maxPixelDistance) {
        return PointHitResult{bestSeriesIndex, bestSampleIndex,
                              bestDataPoint, bestPixelDist};
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
