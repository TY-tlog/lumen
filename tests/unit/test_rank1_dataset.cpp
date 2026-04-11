// Unit tests for Rank1Dataset.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Rank1Dataset.h>
#include <data/Unit.h>

#include <cstdint>

using namespace lumen::data;
using Catch::Matchers::WithinRel;

TEST_CASE("Rank1Dataset double construction", "[rank1]") {
    Rank1Dataset ds(QStringLiteral("voltage"), Unit::parse(QStringLiteral("mV")),
                    std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0});

    REQUIRE(ds.name() == QStringLiteral("voltage"));
    REQUIRE(ds.rank() == 1);
    REQUIRE(ds.shape().size() == 1);
    REQUIRE(ds.shape()[0] == 5);
    REQUIRE(ds.rowCount() == 5);
    REQUIRE(ds.storageMode() == Dataset::StorageMode::InMemory);
}

TEST_CASE("Rank1Dataset doubleData access", "[rank1]") {
    std::vector<double> data = {10.0, 20.0, 30.0};
    Rank1Dataset ds(QStringLiteral("test"), Unit::dimensionless(), data);

    const auto& ref = ds.doubleData();
    REQUIRE(ref.size() == 3);
    REQUIRE_THAT(ref[1], WithinRel(20.0));
}

TEST_CASE("Rank1Dataset valueAt", "[rank1]") {
    Rank1Dataset ds(QStringLiteral("test"), Unit::dimensionless(),
                    std::vector<double>{100.0, 200.0});

    REQUIRE_THAT(ds.valueAt({0}), WithinRel(100.0));
    REQUIRE_THAT(ds.valueAt({1}), WithinRel(200.0));
}

TEST_CASE("Rank1Dataset valueAt wrong rank throws", "[rank1]") {
    Rank1Dataset ds(QStringLiteral("test"), Unit::dimensionless(),
                    std::vector<double>{1.0});

    REQUIRE_THROWS_AS(ds.valueAt({0, 1}), std::invalid_argument);
}

TEST_CASE("Rank1Dataset valueAt out of range throws", "[rank1]") {
    Rank1Dataset ds(QStringLiteral("test"), Unit::dimensionless(),
                    std::vector<double>{1.0, 2.0});

    REQUIRE_THROWS_AS(ds.valueAt({2}), std::out_of_range);
}

TEST_CASE("Rank1Dataset int64 construction and valueAt", "[rank1]") {
    Rank1Dataset ds(QStringLiteral("counts"), Unit::dimensionless(),
                    std::vector<int64_t>{10, 20, 30});

    REQUIRE(ds.rowCount() == 3);
    REQUIRE_THAT(ds.valueAt({1}), WithinRel(20.0));
    REQUIRE(ds.int64Data()[2] == 30);
}

TEST_CASE("Rank1Dataset sizeBytes for doubles", "[rank1]") {
    Rank1Dataset ds(QStringLiteral("test"), Unit::dimensionless(),
                    std::vector<double>{1.0, 2.0, 3.0});

    REQUIRE(ds.sizeBytes() == 3 * sizeof(double));
}

TEST_CASE("Rank1Dataset dimensions returns one dimension", "[rank1]") {
    Rank1Dataset ds(QStringLiteral("test"), Unit::parse(QStringLiteral("V")),
                    std::vector<double>{1.0, 2.0});

    auto dims = ds.dimensions();
    REQUIRE(dims.size() == 1);
    REQUIRE(dims[0].name == QStringLiteral("row"));
    REQUIRE(dims[0].length == 2);
    REQUIRE(dims[0].coordinates.isRegular());
}

TEST_CASE("Rank1Dataset valueUnit preserved", "[rank1]") {
    auto unit = Unit::parse(QStringLiteral("mV"));
    Rank1Dataset ds(QStringLiteral("test"), unit, std::vector<double>{1.0});

    REQUIRE_THAT(ds.valueUnit().scaleToSI(), WithinRel(0.001));
    REQUIRE(ds.valueUnit().isCompatible(unit));
}
