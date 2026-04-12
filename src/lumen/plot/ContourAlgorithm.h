#pragma once

#include <QPointF>

#include <vector>

namespace lumen::data {
class Grid2D;
} // namespace lumen::data

namespace lumen::plot {

/// A single contour line segment between two points at a given level.
struct ContourSegment {
    QPointF a;
    QPointF b;
    double level;
};

/// CONREC-based contour extraction from 2D grids.
///
/// For each 2x2 cell, computes the center value (average of 4 corners),
/// subdivides into 4 triangles, and for each triangle and level checks
/// if the level crosses any edge. If yes, interpolates crossing points
/// and emits a segment.
///
/// Degenerate handling:
/// - Value exactly on level: perturbed by 1e-10
/// - NaN corner: cell skipped
/// - Single-cell grid: no contours produced
class ContourAlgorithm {
public:
    /// Extract contour segments from a Grid2D at the given iso-levels.
    /// @param grid  The 2D grid to contour.
    /// @param levels  Iso-level values to extract contours for.
    /// @return Vector of contour line segments.
    static std::vector<ContourSegment> extract(
        const data::Grid2D& grid,
        const std::vector<double>& levels);

private:
    /// Process one triangle (3 vertices with coordinates and values)
    /// against one level. Emits segments to output.
    static void processTriangle(
        double x0, double y0, double z0,
        double x1, double y1, double z1,
        double x2, double y2, double z2,
        double level,
        std::vector<ContourSegment>& output);

    /// Perturb a value that lies exactly on the level.
    static double perturb(double value, double level);

    /// Linear interpolation between two edge endpoints where the level crosses.
    static QPointF interpolateEdge(
        double x0, double y0, double z0,
        double x1, double y1, double z1,
        double level);
};

} // namespace lumen::plot
