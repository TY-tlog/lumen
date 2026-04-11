// Unit tests for Grid2D.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Unit.h>

using namespace lumen::data;
using Catch::Matchers::WithinRel;

namespace {

Grid2D makeTestGrid()
{
    Dimension dimX{
        QStringLiteral("x"),
        Unit::parse(QStringLiteral("m")),
        3,
        CoordinateArray(0.0, 1.0, 3),
    };
    Dimension dimY{
        QStringLiteral("y"),
        Unit::parse(QStringLiteral("m")),
        2,
        CoordinateArray(0.0, 1.0, 2),
    };

    // 2 rows x 3 cols, row-major:
    // row 0: 1, 2, 3
    // row 1: 4, 5, 6
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    return Grid2D(QStringLiteral("test_grid"), Unit::parse(QStringLiteral("K")),
                  std::move(dimX), std::move(dimY), std::move(data));
}

} // anonymous namespace

TEST_CASE("Grid2D construction and rank", "[grid2d]") {
    auto grid = makeTestGrid();
    REQUIRE(grid.rank() == 2);
    REQUIRE(grid.name() == QStringLiteral("test_grid"));
    REQUIRE(grid.storageMode() == Dataset::StorageMode::InMemory);
}

TEST_CASE("Grid2D shape", "[grid2d]") {
    auto grid = makeTestGrid();
    auto sh = grid.shape();
    REQUIRE(sh.size() == 2);
    REQUIRE(sh[0] == 3); // dimX
    REQUIRE(sh[1] == 2); // dimY
}

TEST_CASE("Grid2D valueAt", "[grid2d]") {
    auto grid = makeTestGrid();

    // data[row * cols + col] → data[0*3+0]=1, data[0*3+1]=2, data[1*3+0]=4
    REQUIRE_THAT(grid.valueAt({0, 0}), WithinRel(1.0)); // x=0, y=0
    REQUIRE_THAT(grid.valueAt({1, 0}), WithinRel(2.0)); // x=1, y=0
    REQUIRE_THAT(grid.valueAt({2, 0}), WithinRel(3.0)); // x=2, y=0
    REQUIRE_THAT(grid.valueAt({0, 1}), WithinRel(4.0)); // x=0, y=1
    REQUIRE_THAT(grid.valueAt({2, 1}), WithinRel(6.0)); // x=2, y=1
}

TEST_CASE("Grid2D valueAt wrong rank throws", "[grid2d]") {
    auto grid = makeTestGrid();
    REQUIRE_THROWS_AS(grid.valueAt({0}), std::invalid_argument);
    REQUIRE_THROWS_AS(grid.valueAt({0, 0, 0}), std::invalid_argument);
}

TEST_CASE("Grid2D valueAt out of range throws", "[grid2d]") {
    auto grid = makeTestGrid();
    REQUIRE_THROWS_AS(grid.valueAt({3, 0}), std::out_of_range);
    REQUIRE_THROWS_AS(grid.valueAt({0, 2}), std::out_of_range);
}

TEST_CASE("Grid2D sizeBytes", "[grid2d]") {
    auto grid = makeTestGrid();
    REQUIRE(grid.sizeBytes() == 6 * sizeof(double));
}

TEST_CASE("Grid2D dimensions", "[grid2d]") {
    auto grid = makeTestGrid();
    auto dims = grid.dimensions();
    REQUIRE(dims.size() == 2);
    REQUIRE(dims[0].name == QStringLiteral("x"));
    REQUIRE(dims[1].name == QStringLiteral("y"));
}

TEST_CASE("Grid2D rejects wrong data size", "[grid2d]") {
    Dimension dimX{
        QStringLiteral("x"), Unit::dimensionless(), 2,
        CoordinateArray(0.0, 1.0, 2),
    };
    Dimension dimY{
        QStringLiteral("y"), Unit::dimensionless(), 3,
        CoordinateArray(0.0, 1.0, 3),
    };

    std::vector<double> wrongSize = {1.0, 2.0, 3.0}; // need 6
    REQUIRE_THROWS_AS(
        Grid2D(QStringLiteral("bad"), Unit::dimensionless(),
               std::move(dimX), std::move(dimY), std::move(wrongSize)),
        std::invalid_argument);
}

TEST_CASE("Grid2D valueUnit", "[grid2d]") {
    auto grid = makeTestGrid();
    REQUIRE(grid.valueUnit().dimensions()[Unit::Temperature] == 1);
}
