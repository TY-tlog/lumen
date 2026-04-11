// Unit tests for Dimension struct.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Unit.h>

using namespace lumen::data;
using Catch::Matchers::WithinRel;

TEST_CASE("Dimension construction with regular coordinates", "[dimension]") {
    Dimension dim{
        QStringLiteral("time"),
        Unit::parse(QStringLiteral("ms")),
        100,
        CoordinateArray(0.0, 0.1, 100),
    };

    REQUIRE(dim.name == QStringLiteral("time"));
    REQUIRE(dim.length == 100);
    REQUIRE(dim.coordinates.isRegular());
    REQUIRE(dim.coordinates.size() == 100);
}

TEST_CASE("Dimension construction with irregular coordinates", "[dimension]") {
    std::vector<double> vals = {0.0, 0.5, 1.5, 4.0};
    Dimension dim{
        QStringLiteral("x"),
        Unit::parse(QStringLiteral("m")),
        4,
        CoordinateArray(vals),
    };

    REQUIRE(dim.name == QStringLiteral("x"));
    REQUIRE(dim.length == 4);
    REQUIRE_FALSE(dim.coordinates.isRegular());
    REQUIRE_THAT(dim.coordinates.valueAt(2), WithinRel(1.5));
}

TEST_CASE("Dimension unit is preserved", "[dimension]") {
    Dimension dim{
        QStringLiteral("voltage"),
        Unit::parse(QStringLiteral("mV")),
        10,
        CoordinateArray(0.0, 1.0, 10),
    };

    REQUIRE_THAT(dim.unit.scaleToSI(), WithinRel(0.001));
    REQUIRE(dim.unit.dimensions()[Unit::Length] == 2); // Volt dimensions
}
