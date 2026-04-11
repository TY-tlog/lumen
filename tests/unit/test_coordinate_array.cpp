// Unit tests for CoordinateArray.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>

using namespace lumen::data;
using Catch::Matchers::WithinRel;

// ===== Regular coordinates =====

TEST_CASE("CoordinateArray regular construction", "[coordinate]") {
    CoordinateArray ca(0.0, 0.5, 10);
    REQUIRE(ca.isRegular());
    REQUIRE(ca.size() == 10);
}

TEST_CASE("CoordinateArray regular valueAt", "[coordinate]") {
    CoordinateArray ca(1.0, 0.25, 5);
    REQUIRE_THAT(ca.valueAt(0), WithinRel(1.0));
    REQUIRE_THAT(ca.valueAt(1), WithinRel(1.25));
    REQUIRE_THAT(ca.valueAt(4), WithinRel(2.0));
}

TEST_CASE("CoordinateArray regular valueAt out of range throws", "[coordinate]") {
    CoordinateArray ca(0.0, 1.0, 3);
    REQUIRE_THROWS_AS(ca.valueAt(3), std::out_of_range);
}

TEST_CASE("CoordinateArray regular indexOf", "[coordinate]") {
    CoordinateArray ca(0.0, 1.0, 10); // 0,1,2,...,9
    auto idx = ca.indexOf(4.3);
    REQUIRE(idx.has_value());
    REQUIRE(idx.value() == 4);
}

TEST_CASE("CoordinateArray regular indexOf at boundary", "[coordinate]") {
    CoordinateArray ca(0.0, 1.0, 5); // 0,1,2,3,4
    auto idx = ca.indexOf(4.9);
    REQUIRE(idx.has_value());
    REQUIRE(idx.value() == 4); // clamped to last valid index
}

// ===== Irregular coordinates =====

TEST_CASE("CoordinateArray irregular construction", "[coordinate]") {
    std::vector<double> vals = {0.0, 1.5, 3.0, 7.0};
    CoordinateArray ca(vals);
    REQUIRE_FALSE(ca.isRegular());
    REQUIRE(ca.size() == 4);
}

TEST_CASE("CoordinateArray irregular valueAt", "[coordinate]") {
    std::vector<double> vals = {10.0, 20.0, 30.0};
    CoordinateArray ca(vals);
    REQUIRE_THAT(ca.valueAt(0), WithinRel(10.0));
    REQUIRE_THAT(ca.valueAt(2), WithinRel(30.0));
}

TEST_CASE("CoordinateArray irregular valueAt out of range throws", "[coordinate]") {
    std::vector<double> vals = {1.0, 2.0};
    CoordinateArray ca(vals);
    REQUIRE_THROWS_AS(ca.valueAt(2), std::out_of_range);
}

TEST_CASE("CoordinateArray irregular indexOf nearest", "[coordinate]") {
    std::vector<double> vals = {0.0, 1.0, 5.0, 10.0};
    CoordinateArray ca(vals);
    auto idx = ca.indexOf(4.0);
    REQUIRE(idx.has_value());
    REQUIRE(idx.value() == 2); // 5.0 is nearest to 4.0
}

TEST_CASE("CoordinateArray irregular indexOf exact", "[coordinate]") {
    std::vector<double> vals = {0.0, 1.0, 2.0, 3.0};
    CoordinateArray ca(vals);
    auto idx = ca.indexOf(2.0);
    REQUIRE(idx.has_value());
    REQUIRE(idx.value() == 2);
}

TEST_CASE("CoordinateArray empty indexOf returns nullopt", "[coordinate]") {
    std::vector<double> vals;
    CoordinateArray ca(vals);
    REQUIRE(ca.size() == 0);
    auto idx = ca.indexOf(1.0);
    REQUIRE_FALSE(idx.has_value());
}
