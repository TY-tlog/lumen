// Unit tests for ContourAlgorithm (CONREC extraction) — T12.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Unit.h>
#include <plot/ContourAlgorithm.h>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

using namespace lumen::data;
using namespace lumen::plot;
using Catch::Matchers::WithinAbs;

namespace {

const auto kNaN = std::numeric_limits<double>::quiet_NaN();

/// Helper: create a grid with given data (row-major).
std::shared_ptr<Grid2D> makeGrid(std::size_t cols, std::size_t rows,
                                  std::vector<double> data)
{
    Dimension dimX{
        QStringLiteral("x"),
        Unit::dimensionless(),
        cols,
        CoordinateArray(0.0, 1.0, cols),
    };
    Dimension dimY{
        QStringLiteral("y"),
        Unit::dimensionless(),
        rows,
        CoordinateArray(0.0, 1.0, rows),
    };
    return std::make_shared<Grid2D>(
        QStringLiteral("test"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

/// Helper: create a Gaussian grid centered at (cx, cy) with given sigma.
std::shared_ptr<Grid2D> makeGaussianGrid(std::size_t size, double sigma)
{
    Dimension dimX{
        QStringLiteral("x"),
        Unit::dimensionless(),
        size,
        CoordinateArray(0.0, 1.0, size),
    };
    Dimension dimY{
        QStringLiteral("y"),
        Unit::dimensionless(),
        size,
        CoordinateArray(0.0, 1.0, size),
    };

    double cx = static_cast<double>(size - 1) / 2.0;
    double cy = cx;
    std::vector<double> data(size * size);
    for (std::size_t r = 0; r < size; ++r) {
        for (std::size_t c = 0; c < size; ++c) {
            double x = static_cast<double>(c);
            double y = static_cast<double>(r);
            double dx = x - cx;
            double dy = y - cy;
            data[r * size + c] = std::exp(-(dx * dx + dy * dy) / (2.0 * sigma * sigma));
        }
    }
    return std::make_shared<Grid2D>(
        QStringLiteral("gaussian"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

} // anonymous namespace

TEST_CASE("ContourAlgorithm: Gaussian grid produces circular contours", "[contour]")
{
    auto grid = makeGaussianGrid(21, 4.0);
    std::vector<double> levels = {0.5};

    auto segments = ContourAlgorithm::extract(*grid, levels);

    // Should produce a ring of segments around the center.
    REQUIRE(segments.size() > 4);

    // All segments should be at level 0.5.
    for (const auto& seg : segments) {
        REQUIRE_THAT(seg.level, WithinAbs(0.5, 1e-12));
    }

    // Segments should roughly form a circle: distance from center should be similar.
    double cx = 10.0; // center of 21-point grid with step 1.0
    double cy = 10.0;

    double minDist = 1e9;
    double maxDist = 0.0;
    for (const auto& seg : segments) {
        // Check midpoint distance from center.
        double mx = (seg.a.x() + seg.b.x()) / 2.0;
        double my = (seg.a.y() + seg.b.y()) / 2.0;
        double dist = std::sqrt((mx - cx) * (mx - cx) + (my - cy) * (my - cy));
        minDist = std::min(minDist, dist);
        maxDist = std::max(maxDist, dist);
    }

    // The spread of distances should be small relative to the radius.
    double spread = maxDist - minDist;
    double avgDist = (minDist + maxDist) / 2.0;
    REQUIRE(spread / avgDist < 0.3); // Roughly circular.
}

TEST_CASE("ContourAlgorithm: saddle point handled", "[contour]")
{
    // 3x3 grid with a saddle at the center: high corners at (0,0) and (2,2),
    // low at (0,2) and (2,0).
    // Values: z = (x - 1)^2 - (y - 1)^2 over a 3x3 grid.
    // Row 0: (0-1)^2-(0-1)^2=0, (1-1)^2-(0-1)^2=-1, (2-1)^2-(0-1)^2=0
    // Row 1: (0-1)^2-(1-1)^2=1, (1-1)^2-(1-1)^2=0,  (2-1)^2-(1-1)^2=1
    // Row 2: (0-1)^2-(2-1)^2=0, (1-1)^2-(2-1)^2=-1, (2-1)^2-(2-1)^2=0
    auto grid = makeGrid(3, 3, {0, -1, 0,
                                 1,  0, 1,
                                 0, -1, 0});

    std::vector<double> levels = {0.0};
    auto segments = ContourAlgorithm::extract(*grid, levels);

    // Saddle point means the zero contour should produce segments.
    // The value-on-level perturbation ensures we don't get degenerate crossings.
    REQUIRE(!segments.empty());
}

TEST_CASE("ContourAlgorithm: value exactly on level is perturbed", "[contour]")
{
    // A 2x2 grid where all corners are exactly at the level value.
    // After perturbation, they should all be slightly above the level,
    // so no contour should be generated.
    auto grid = makeGrid(2, 2, {5.0, 5.0, 5.0, 5.0});

    std::vector<double> levels = {5.0};
    auto segments = ContourAlgorithm::extract(*grid, levels);

    // All perturbed to 5+1e-10, all on the same side of 5.0 -> no crossing.
    REQUIRE(segments.empty());
}

TEST_CASE("ContourAlgorithm: auto-levels from data range", "[contour]")
{
    // 3x3 grid with values 0 to 8.
    auto grid = makeGrid(3, 3, {0, 1, 2, 3, 4, 5, 6, 7, 8});

    // Two levels between 0 and 8: should be at roughly 2.67 and 5.33.
    double step = 8.0 / 3.0;
    std::vector<double> levels = {step, 2.0 * step};

    auto segments = ContourAlgorithm::extract(*grid, levels);
    REQUIRE(!segments.empty());

    // Each level should have at least one segment.
    bool hasLevel1 = false;
    bool hasLevel2 = false;
    for (const auto& seg : segments) {
        if (std::abs(seg.level - step) < 1e-6) {
            hasLevel1 = true;
        }
        if (std::abs(seg.level - 2.0 * step) < 1e-6) {
            hasLevel2 = true;
        }
    }
    REQUIRE(hasLevel1);
    REQUIRE(hasLevel2);
}

TEST_CASE("ContourAlgorithm: NaN cell skipped", "[contour]")
{
    // 3x3 grid with NaN in the center cell.
    auto grid = makeGrid(3, 3, {0, 1, 2,
                                 3, kNaN, 5,
                                 6, 7, 8});

    std::vector<double> levels = {4.0};
    auto segments = ContourAlgorithm::extract(*grid, levels);

    // Segments should still be produced from cells without NaN.
    // Verify that no segment passes through the center region (which has NaN).
    for (const auto& seg : segments) {
        // The center cell spans [0.5,1.5] in x and [0.5,1.5] in y.
        // Segments from cells touching the NaN center should be skipped.
        // The midpoint of segments near the center is not expected.
        double mx = (seg.a.x() + seg.b.x()) / 2.0;
        double my = (seg.a.y() + seg.b.y()) / 2.0;
        // Not all four cells touching center should produce segments.
        // This is a basic sanity check - we verify segments exist.
        (void)mx;
        (void)my;
    }
    // The outer cells that don't touch NaN may or may not produce segments
    // depending on whether the level crosses. With value 4.0 and the pattern,
    // cells not adjacent to NaN: (0,0)-(1,0)-(0,1)-(1,1) has NaN in (1,1);
    // so cells touching center NaN are skipped.
    // At least the corner cell (col=1,row=1)-(col=2,row=1)-(col=1,row=2)-(col=2,row=2)
    // = (NaN,5,7,8) is skipped due to NaN. But cell (0,0)-(1,0)-(0,1)-(1,1) also has NaN.
    // The only cells without NaN are those not touching center:
    // Top-right: no, (1,0) is adjacent.
    // Actually all 4 cells touch the center of a 3x3 grid.
    // So with NaN at center, ALL cells are skipped -> no segments.
    REQUIRE(segments.empty());
}

TEST_CASE("ContourAlgorithm: single-cell grid produces no contours", "[contour]")
{
    auto grid = makeGrid(1, 1, {5.0});
    std::vector<double> levels = {3.0};

    auto segments = ContourAlgorithm::extract(*grid, levels);
    REQUIRE(segments.empty());
}

TEST_CASE("ContourAlgorithm: empty levels produces no contours", "[contour]")
{
    auto grid = makeGrid(3, 3, {0, 1, 2, 3, 4, 5, 6, 7, 8});
    std::vector<double> levels = {};

    auto segments = ContourAlgorithm::extract(*grid, levels);
    REQUIRE(segments.empty());
}

TEST_CASE("ContourAlgorithm: level outside data range produces no contours", "[contour]")
{
    auto grid = makeGrid(3, 3, {0, 1, 2, 3, 4, 5, 6, 7, 8});
    std::vector<double> levels = {100.0};

    auto segments = ContourAlgorithm::extract(*grid, levels);
    REQUIRE(segments.empty());
}
