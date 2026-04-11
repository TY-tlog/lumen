// Unit tests for Unit::convert() and isCompatible().

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Unit.h>

using namespace lumen::data;
using Catch::Matchers::WithinRel;

TEST_CASE("Unit convert mV to V", "[unit][conversion]") {
    auto mV = Unit::parse(QStringLiteral("mV"));
    auto V = Unit::parse(QStringLiteral("V"));

    // 100 mV = 0.1 V
    REQUIRE_THAT(mV.convert(100.0, V), WithinRel(0.1));
}

TEST_CASE("Unit convert V to mV", "[unit][conversion]") {
    auto V = Unit::parse(QStringLiteral("V"));
    auto mV = Unit::parse(QStringLiteral("mV"));

    // 0.1 V = 100 mV
    REQUIRE_THAT(V.convert(0.1, mV), WithinRel(100.0));
}

TEST_CASE("Unit convert ms to s", "[unit][conversion]") {
    auto ms = Unit::parse(QStringLiteral("ms"));
    auto s = Unit::parse(QStringLiteral("s"));

    // 500 ms = 0.5 s
    REQUIRE_THAT(ms.convert(500.0, s), WithinRel(0.5));
}

TEST_CASE("Unit convert s to ms", "[unit][conversion]") {
    auto s = Unit::parse(QStringLiteral("s"));
    auto ms = Unit::parse(QStringLiteral("ms"));

    // 1.5 s = 1500 ms
    REQUIRE_THAT(s.convert(1.5, ms), WithinRel(1500.0));
}

TEST_CASE("Unit convert mm to m", "[unit][conversion]") {
    auto mm = Unit::parse(QStringLiteral("mm"));
    auto m = Unit::parse(QStringLiteral("m"));

    // 2500 mm = 2.5 m
    REQUIRE_THAT(mm.convert(2500.0, m), WithinRel(2.5));
}

TEST_CASE("Unit convert same unit is identity", "[unit][conversion]") {
    auto V = Unit::parse(QStringLiteral("V"));
    REQUIRE_THAT(V.convert(42.0, V), WithinRel(42.0));
}

TEST_CASE("Unit convert incompatible throws", "[unit][conversion]") {
    auto V = Unit::parse(QStringLiteral("V"));
    auto s = Unit::parse(QStringLiteral("s"));
    REQUIRE_THROWS_AS(V.convert(1.0, s), std::invalid_argument);
}

TEST_CASE("Unit isCompatible same dimensions", "[unit][conversion]") {
    auto mV = Unit::parse(QStringLiteral("mV"));
    auto V = Unit::parse(QStringLiteral("V"));
    REQUIRE(mV.isCompatible(V));
}

TEST_CASE("Unit isCompatible different dimensions", "[unit][conversion]") {
    auto V = Unit::parse(QStringLiteral("V"));
    auto A = Unit::parse(QStringLiteral("A"));
    REQUIRE_FALSE(V.isCompatible(A));
}

TEST_CASE("Unit dimensionless is self-compatible", "[unit][conversion]") {
    auto dl = Unit::dimensionless();
    REQUIRE(dl.isCompatible(dl));
}

TEST_CASE("Unit convert nA to pA", "[unit][conversion]") {
    auto nA = Unit::parse(QStringLiteral("nA"));
    auto pA = Unit::parse(QStringLiteral("pA"));

    // 1 nA = 1000 pA
    REQUIRE_THAT(nA.convert(1.0, pA), WithinRel(1000.0));
}
