// Spike test: can we subtract two Grid2D datasets with different units/grids?
// This tests the health of the Phase 6 Dataset abstraction before Phase 7.
//
// Scenarios:
// 1. Same grid, same units → simple element-wise subtraction
// 2. Same grid, compatible units (mV vs V) → needs unit conversion
// 3. Different grid spacing → needs resampling/interpolation
// 4. Incompatible units (V vs m) → should reject
//
// Expected result: if the abstraction is healthy, scenarios 1-2 should be
// straightforward. Scenario 3 should be possible with explicit resampling.
// Scenario 4 should be caught by unit compatibility check.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Unit.h>

#include <cmath>
#include <vector>

using namespace lumen::data;

namespace {

// Helper: create a Grid2D with given size, unit, and fill value.
std::shared_ptr<Grid2D> makeGrid(const QString& name, Unit unit,
                                  std::size_t w, std::size_t h,
                                  double xStart, double xStep,
                                  double yStart, double yStep,
                                  double fillValue)
{
    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), w,
                   CoordinateArray(xStart, xStep, w)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), h,
                   CoordinateArray(yStart, yStep, h)};

    std::vector<double> values(w * h, fillValue);
    return std::make_shared<Grid2D>(name, unit, std::move(dimX),
                                     std::move(dimY), std::move(values));
}

// Manual element-wise subtraction (what Phase 7 would need to implement).
std::vector<double> subtractGrids(const Grid2D& a, const Grid2D& b)
{
    auto shapeA = a.shape();
    auto shapeB = b.shape();
    REQUIRE(shapeA == shapeB);  // Same grid size required

    std::vector<double> result(shapeA[0] * shapeA[1]);
    for (std::size_t i = 0; i < shapeA[0]; ++i) {
        for (std::size_t j = 0; j < shapeA[1]; ++j) {
            result[i * shapeA[1] + j] =
                a.valueAt({i, j}) - b.valueAt({i, j});
        }
    }
    return result;
}

}  // namespace

TEST_CASE("Spike: same grid, same units — element-wise subtraction", "[spike]") {
    auto gridA = makeGrid("A", Unit::parse("mV"), 4, 4, 0, 1, 0, 1, 10.0);
    auto gridB = makeGrid("B", Unit::parse("mV"), 4, 4, 0, 1, 0, 1, 3.0);

    // Subtraction: straightforward, same shape.
    auto result = subtractGrids(*gridA, *gridB);

    REQUIRE(result.size() == 16);
    for (double v : result) {
        CHECK(v == Catch::Approx(7.0));
    }

    // Result unit: mV - mV = mV (same unit, subtraction preserves it).
    Unit resultUnit = gridA->valueUnit();  // stays mV
    CHECK(resultUnit.isCompatible(gridB->valueUnit()));
    CHECK(resultUnit.symbol() == "mV");
}

TEST_CASE("Spike: same grid, compatible units (mV vs V) — needs conversion", "[spike]") {
    auto gridA = makeGrid("A", Unit::parse("mV"), 4, 4, 0, 1, 0, 1, 100.0);  // 100 mV
    auto gridB = makeGrid("B", Unit::parse("V"),  4, 4, 0, 1, 0, 1, 0.05);   // 0.05 V = 50 mV

    // Units are compatible (both voltage).
    CHECK(gridA->valueUnit().isCompatible(gridB->valueUnit()));

    // To subtract, convert B to A's units first.
    double conversionFactor = gridB->valueUnit().convert(1.0, gridA->valueUnit());
    // 1 V = 1000 mV, so factor should be 1000.
    CHECK(conversionFactor == Catch::Approx(1000.0));

    // Manual subtraction with conversion:
    auto shapeA = gridA->shape();
    std::vector<double> result(shapeA[0] * shapeA[1]);
    for (std::size_t i = 0; i < shapeA[0]; ++i) {
        for (std::size_t j = 0; j < shapeA[1]; ++j) {
            double aVal = gridA->valueAt({i, j});
            double bVal = gridB->valueAt({i, j}) * conversionFactor;
            result[i * shapeA[1] + j] = aVal - bVal;
        }
    }

    // 100 mV - 50 mV = 50 mV
    for (double v : result) {
        CHECK(v == Catch::Approx(50.0));
    }
}

TEST_CASE("Spike: different grid spacing — resampling NOT built-in", "[spike]") {
    // Grid A: 4x4, spacing 1.0
    auto gridA = makeGrid("A", Unit::parse("mV"), 4, 4, 0, 1.0, 0, 1.0, 10.0);
    // Grid B: 8x8, spacing 0.5 (finer grid)
    auto gridB = makeGrid("B", Unit::parse("mV"), 8, 8, 0, 0.5, 0, 0.5, 5.0);

    // Shapes differ — direct subtraction is impossible.
    CHECK(gridA->shape() != gridB->shape());

    // The abstraction does NOT provide resampling.
    // Phase 7+ would need to implement:
    //   1. Interpolation of gridB onto gridA's coordinates, or
    //   2. A common resampled grid for both.
    // This is NOT a failure of the abstraction — it's expected.
    // The coordinate arrays provide the information needed for resampling.

    auto dimsA = gridA->dimensions();
    auto dimsB = gridB->dimensions();

    // We CAN access coordinate values for both grids.
    CHECK(dimsA[0].coordinates.valueAt(0) == Catch::Approx(0.0));
    CHECK(dimsA[0].coordinates.valueAt(1) == Catch::Approx(1.0));
    CHECK(dimsB[0].coordinates.valueAt(0) == Catch::Approx(0.0));
    CHECK(dimsB[0].coordinates.valueAt(1) == Catch::Approx(0.5));

    // Verdict: resampling is NOT in Dataset. It must be in a separate
    // analysis/transform layer (Phase 7+). The abstraction provides
    // the coordinate data needed to implement it.
    SUCCEED("Resampling not built-in; coordinate data available for Phase 7");
}

TEST_CASE("Spike: incompatible units (V vs m) — caught by isCompatible", "[spike]") {
    auto gridA = makeGrid("A", Unit::parse("V"), 4, 4, 0, 1, 0, 1, 10.0);
    auto gridB = makeGrid("B", Unit::parse("m"), 4, 4, 0, 1, 0, 1, 5.0);

    // Units are incompatible (voltage vs length).
    CHECK_FALSE(gridA->valueUnit().isCompatible(gridB->valueUnit()));

    // The abstraction correctly rejects this combination.
    // Phase 7 analysis code should check isCompatible() before arithmetic.
    SUCCEED("Incompatible units correctly detected");
}

TEST_CASE("Spike: unit arithmetic — V/s for differentiation", "[spike]") {
    Unit volts = Unit::parse("V");
    Unit seconds = Unit::parse("s");

    // dV/dt has units V/s
    Unit dvdt = volts / seconds;
    CHECK(dvdt.symbol().contains("V"));
    // The symbol might be "V/s" or "V*s^-1" depending on implementation.

    // V/s is NOT compatible with V (different dimensions).
    CHECK_FALSE(dvdt.isCompatible(volts));

    // V/s IS compatible with mV/ms (same dimensions, different scale).
    Unit millivolts = Unit::parse("mV");
    Unit milliseconds = Unit::parse("ms");
    Unit dvdt_milli = millivolts / milliseconds;

    CHECK(dvdt.isCompatible(dvdt_milli));

    // Conversion: 1 mV/ms = 1 V/s (milli cancels)
    double factor = dvdt_milli.convert(1.0, dvdt);
    CHECK(factor == Catch::Approx(1.0));
}

TEST_CASE("Spike: Grid2D valueAt performance — 256x256 full scan", "[spike]") {
    auto grid = makeGrid("perf", Unit::dimensionless(), 256, 256,
                          0, 1, 0, 1, 42.0);

    // Full scan: 65536 valueAt calls.
    double sum = 0.0;
    auto shape = grid->shape();
    for (std::size_t i = 0; i < shape[0]; ++i) {
        for (std::size_t j = 0; j < shape[1]; ++j) {
            sum += grid->valueAt({i, j});
        }
    }

    CHECK(sum == Catch::Approx(42.0 * 256 * 256));
    // If this test takes > 100ms, valueAt is too slow for Phase 7 rendering.
}
