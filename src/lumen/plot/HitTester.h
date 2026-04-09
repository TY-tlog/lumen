#pragma once

#include <QPointF>

#include <optional>

namespace lumen::plot {

class PlotScene;
class CoordinateMapper;

/// Performs spatial hit-testing against line series in pixel space.
///
/// Used by InteractionController to determine which series (if any) the
/// user clicked on. Lives in plot/ (UI-independent) so it can be unit-tested
/// without QWidget dependencies.
class HitTester {
public:
    struct HitResult {
        int seriesIndex = -1;
        double pixelDistance = 0.0;
    };

    /// Find the nearest visible LineSeries to a pixel position.
    /// Returns nullopt if no series is within tolerancePx.
    static std::optional<HitResult> hitTest(
        const PlotScene& scene,
        const CoordinateMapper& mapper,
        QPointF pixelPos,
        double tolerancePx = 5.0);

private:
    /// Compute perpendicular distance from point to line segment.
    static double pointToSegmentDistance(QPointF point, QPointF segA, QPointF segB);
};

}  // namespace lumen::plot
