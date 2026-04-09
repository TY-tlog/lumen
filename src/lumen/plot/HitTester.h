#pragma once

#include <QPointF>
#include <QRectF>
#include <QSizeF>

#include <optional>

namespace lumen::plot {

class PlotScene;
class CoordinateMapper;

/// Which non-series region was hit. Checked in precedence order:
/// Title > Legend > XAxis > YAxis > PlotArea > None.
enum class HitKind { None, XAxis, YAxis, Title, Legend, PlotArea };

/// Result of a non-series region hit test.
struct RegionHitResult {
    HitKind kind = HitKind::None;
};

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

    /// Determine which non-series element (axis, title, legend, plot area)
    /// contains the given pixel position. The widget size is needed to
    /// compute regions outside the plot area (axes, title).
    ///
    /// Precedence order (per ADR-024): Title > Legend > XAxis > YAxis > PlotArea > None.
    static RegionHitResult hitNonSeriesElement(
        const PlotScene& scene,
        QSizeF widgetSize,
        QPointF pixelPos);

private:
    /// Compute perpendicular distance from point to line segment.
    static double pointToSegmentDistance(QPointF point, QPointF segA, QPointF segB);

    /// Compute the approximate legend rect inside the plot area.
    static QRectF computeLegendRect(const PlotScene& scene, QRectF plotArea);
};

}  // namespace lumen::plot
