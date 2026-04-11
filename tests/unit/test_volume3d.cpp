// Unit tests for Volume3D.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Unit.h>
#include <data/Volume3D.h>

using namespace lumen::data;
using Catch::Matchers::WithinRel;

namespace {

Volume3D makeTestVolume()
{
    Dimension dimX{
        QStringLiteral("x"), Unit::parse(QStringLiteral("m")), 2,
        CoordinateArray(0.0, 1.0, 2),
    };
    Dimension dimY{
        QStringLiteral("y"), Unit::parse(QStringLiteral("m")), 3,
        CoordinateArray(0.0, 1.0, 3),
    };
    Dimension dimZ{
        QStringLiteral("z"), Unit::parse(QStringLiteral("m")), 2,
        CoordinateArray(0.0, 1.0, 2),
    };

    // 2 x 3 x 2 = 12 elements
    // Index formula: z * (dimY * dimX) + y * dimX + x
    std::vector<double> data(12);
    for (std::size_t z = 0; z < 2; ++z) {
        for (std::size_t y = 0; y < 3; ++y) {
            for (std::size_t x = 0; x < 2; ++x) {
                data[z * 6 + y * 2 + x]
                    = static_cast<double>(x * 100 + y * 10 + z);
            }
        }
    }

    return Volume3D(QStringLiteral("test_vol"), Unit::parse(QStringLiteral("Pa")),
                    std::move(dimX), std::move(dimY), std::move(dimZ),
                    std::move(data));
}

} // anonymous namespace

TEST_CASE("Volume3D construction and rank", "[volume3d]") {
    auto vol = makeTestVolume();
    REQUIRE(vol.rank() == 3);
    REQUIRE(vol.name() == QStringLiteral("test_vol"));
    REQUIRE(vol.storageMode() == Dataset::StorageMode::InMemory);
}

TEST_CASE("Volume3D shape", "[volume3d]") {
    auto vol = makeTestVolume();
    auto sh = vol.shape();
    REQUIRE(sh.size() == 3);
    REQUIRE(sh[0] == 2); // dimX
    REQUIRE(sh[1] == 3); // dimY
    REQUIRE(sh[2] == 2); // dimZ
}

TEST_CASE("Volume3D valueAt", "[volume3d]") {
    auto vol = makeTestVolume();

    // x=0, y=0, z=0 → 0*100 + 0*10 + 0 = 0
    REQUIRE(vol.valueAt({0, 0, 0}) == 0.0);

    // x=1, y=0, z=0 → 1*100 + 0*10 + 0 = 100
    REQUIRE_THAT(vol.valueAt({1, 0, 0}), WithinRel(100.0));

    // x=0, y=2, z=1 → 0*100 + 2*10 + 1 = 21
    REQUIRE_THAT(vol.valueAt({0, 2, 1}), WithinRel(21.0));

    // x=1, y=1, z=1 → 1*100 + 1*10 + 1 = 111
    REQUIRE_THAT(vol.valueAt({1, 1, 1}), WithinRel(111.0));
}

TEST_CASE("Volume3D valueAt wrong rank throws", "[volume3d]") {
    auto vol = makeTestVolume();
    REQUIRE_THROWS_AS(vol.valueAt({0, 0}), std::invalid_argument);
    REQUIRE_THROWS_AS(vol.valueAt({0, 0, 0, 0}), std::invalid_argument);
}

TEST_CASE("Volume3D valueAt out of range throws", "[volume3d]") {
    auto vol = makeTestVolume();
    REQUIRE_THROWS_AS(vol.valueAt({2, 0, 0}), std::out_of_range);
    REQUIRE_THROWS_AS(vol.valueAt({0, 3, 0}), std::out_of_range);
    REQUIRE_THROWS_AS(vol.valueAt({0, 0, 2}), std::out_of_range);
}

TEST_CASE("Volume3D sizeBytes", "[volume3d]") {
    auto vol = makeTestVolume();
    REQUIRE(vol.sizeBytes() == 12 * sizeof(double));
}

TEST_CASE("Volume3D dimensions", "[volume3d]") {
    auto vol = makeTestVolume();
    auto dims = vol.dimensions();
    REQUIRE(dims.size() == 3);
    REQUIRE(dims[0].name == QStringLiteral("x"));
    REQUIRE(dims[1].name == QStringLiteral("y"));
    REQUIRE(dims[2].name == QStringLiteral("z"));
}

TEST_CASE("Volume3D rejects wrong data size", "[volume3d]") {
    Dimension dimX{
        QStringLiteral("x"), Unit::dimensionless(), 2,
        CoordinateArray(0.0, 1.0, 2),
    };
    Dimension dimY{
        QStringLiteral("y"), Unit::dimensionless(), 2,
        CoordinateArray(0.0, 1.0, 2),
    };
    Dimension dimZ{
        QStringLiteral("z"), Unit::dimensionless(), 2,
        CoordinateArray(0.0, 1.0, 2),
    };

    std::vector<double> wrongSize = {1.0, 2.0, 3.0}; // need 8
    REQUIRE_THROWS_AS(
        Volume3D(QStringLiteral("bad"), Unit::dimensionless(),
                 std::move(dimX), std::move(dimY), std::move(dimZ),
                 std::move(wrongSize)),
        std::invalid_argument);
}
