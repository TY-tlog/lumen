#include "plot/HitTester.h"

#include "plot/CoordinateMapper.h"
#include "plot/LineSeries.h"
#include "plot/PlotScene.h"

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

}  // namespace lumen::plot
