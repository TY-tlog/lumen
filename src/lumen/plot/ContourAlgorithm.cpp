#include "plot/ContourAlgorithm.h"

#include "data/Grid2D.h"

#include <cmath>
#include <cstddef>

namespace lumen::plot {

namespace {

constexpr double kPerturbation = 1e-10;

} // anonymous namespace

double ContourAlgorithm::perturb(double value, double level)
{
    if (value == level) {
        return value + kPerturbation;
    }
    return value;
}

QPointF ContourAlgorithm::interpolateEdge(
    double x0, double y0, double z0,
    double x1, double y1, double z1,
    double level)
{
    double dz = z1 - z0;
    if (std::abs(dz) < 1e-15) {
        // Degenerate: both endpoints at same value. Return midpoint.
        return QPointF((x0 + x1) / 2.0, (y0 + y1) / 2.0);
    }
    double t = (level - z0) / dz;
    return QPointF(x0 + t * (x1 - x0), y0 + t * (y1 - y0));
}

void ContourAlgorithm::processTriangle(
    double x0, double y0, double z0,
    double x1, double y1, double z1,
    double x2, double y2, double z2,
    double level,
    std::vector<ContourSegment>& output)
{
    // Perturb values that lie exactly on the level.
    z0 = perturb(z0, level);
    z1 = perturb(z1, level);
    z2 = perturb(z2, level);

    // Classify each vertex as above or below the level.
    bool above0 = z0 > level;
    bool above1 = z1 > level;
    bool above2 = z2 > level;

    // If all same side, no crossing.
    if (above0 == above1 && above1 == above2) {
        return;
    }

    // Find the two crossing points. The level crosses exactly two edges
    // of the triangle (since we perturbed away from exact-on-level).
    // We collect crossing points from all three edges.
    std::vector<QPointF> crossings;
    crossings.reserve(2);

    // Edge 0-1
    if (above0 != above1) {
        crossings.push_back(interpolateEdge(x0, y0, z0, x1, y1, z1, level));
    }
    // Edge 1-2
    if (above1 != above2) {
        crossings.push_back(interpolateEdge(x1, y1, z1, x2, y2, z2, level));
    }
    // Edge 0-2
    if (above0 != above2) {
        crossings.push_back(interpolateEdge(x0, y0, z0, x2, y2, z2, level));
    }

    if (crossings.size() >= 2) {
        output.push_back({crossings[0], crossings[1], level});
    }
}

std::vector<ContourSegment> ContourAlgorithm::extract(
    const data::Grid2D& grid,
    const std::vector<double>& levels)
{
    std::vector<ContourSegment> result;

    auto shape = grid.shape();
    std::size_t cols = shape[0];
    std::size_t rows = shape[1];

    // Need at least a 2x2 grid to form cells.
    if (cols < 2 || rows < 2) {
        return result;
    }

    auto dims = grid.dimensions();

    // Iterate over each 2x2 cell.
    for (std::size_t r = 0; r < rows - 1; ++r) {
        for (std::size_t c = 0; c < cols - 1; ++c) {
            // Corner values: (col, row) -> value
            double z00 = grid.valueAt({c, r});         // bottom-left
            double z10 = grid.valueAt({c + 1, r});     // bottom-right
            double z01 = grid.valueAt({c, r + 1});     // top-left
            double z11 = grid.valueAt({c + 1, r + 1}); // top-right

            // Skip cell if any corner is NaN.
            if (std::isnan(z00) || std::isnan(z10) ||
                std::isnan(z01) || std::isnan(z11)) {
                continue;
            }

            // Corner coordinates in data space.
            double x0 = dims[0].coordinates.valueAt(c);
            double x1 = dims[0].coordinates.valueAt(c + 1);
            double y0 = dims[1].coordinates.valueAt(r);
            double y1 = dims[1].coordinates.valueAt(r + 1);

            // Center of the cell (CONREC subdivision).
            double xc = (x0 + x1) / 2.0;
            double yc = (y0 + y1) / 2.0;
            double zc = (z00 + z10 + z01 + z11) / 4.0;

            // Four triangles sharing the center point:
            // Triangle 1: bottom-left, bottom-right, center
            // Triangle 2: bottom-right, top-right, center
            // Triangle 3: top-right, top-left, center
            // Triangle 4: top-left, bottom-left, center

            for (double level : levels) {
                // Triangle 1: (x0,y0,z00), (x1,y0,z10), (xc,yc,zc)
                processTriangle(x0, y0, z00, x1, y0, z10, xc, yc, zc,
                                level, result);
                // Triangle 2: (x1,y0,z10), (x1,y1,z11), (xc,yc,zc)
                processTriangle(x1, y0, z10, x1, y1, z11, xc, yc, zc,
                                level, result);
                // Triangle 3: (x1,y1,z11), (x0,y1,z01), (xc,yc,zc)
                processTriangle(x1, y1, z11, x0, y1, z01, xc, yc, zc,
                                level, result);
                // Triangle 4: (x0,y1,z01), (x0,y0,z00), (xc,yc,zc)
                processTriangle(x0, y1, z01, x0, y0, z00, xc, yc, zc,
                                level, result);
            }
        }
    }

    return result;
}

} // namespace lumen::plot
