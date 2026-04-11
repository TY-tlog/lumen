// Unit tests for Unit::parse().

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Unit.h>

using namespace lumen::data;
using Catch::Matchers::WithinRel;

// ===== Basic base unit parsing =====

TEST_CASE("Unit parse bare meter", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("m"));
    REQUIRE(u.dimensions()[Unit::Length] == 1);
    REQUIRE(u.dimensions()[Unit::Mass] == 0);
    REQUIRE(u.dimensions()[Unit::Time] == 0);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(1.0));
}

TEST_CASE("Unit parse volt", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("V"));
    // Volt = kg*m^2/(A*s^3) → [2,1,-3,-1,0,0,0]
    REQUIRE(u.dimensions()[Unit::Length] == 2);
    REQUIRE(u.dimensions()[Unit::Mass] == 1);
    REQUIRE(u.dimensions()[Unit::Time] == -3);
    REQUIRE(u.dimensions()[Unit::Current] == -1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(1.0));
}

TEST_CASE("Unit parse ampere", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("A"));
    REQUIRE(u.dimensions()[Unit::Current] == 1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(1.0));
}

TEST_CASE("Unit parse second", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("s"));
    REQUIRE(u.dimensions()[Unit::Time] == 1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(1.0));
}

TEST_CASE("Unit parse hertz", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("Hz"));
    REQUIRE(u.dimensions()[Unit::Time] == -1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(1.0));
}

TEST_CASE("Unit parse kelvin", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("K"));
    REQUIRE(u.dimensions()[Unit::Temperature] == 1);
}

// ===== Prefixed units =====

TEST_CASE("Unit parse millivolt mV", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("mV"));
    // Same dims as V but scale = 0.001
    REQUIRE(u.dimensions()[Unit::Length] == 2);
    REQUIRE(u.dimensions()[Unit::Mass] == 1);
    REQUIRE(u.dimensions()[Unit::Time] == -3);
    REQUIRE(u.dimensions()[Unit::Current] == -1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(0.001));
}

TEST_CASE("Unit parse millimeter mm", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("mm"));
    REQUIRE(u.dimensions()[Unit::Length] == 1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(0.001));
}

TEST_CASE("Unit parse millisecond ms", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("ms"));
    REQUIRE(u.dimensions()[Unit::Time] == 1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(0.001));
}

TEST_CASE("Unit parse nanoampere nA", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("nA"));
    REQUIRE(u.dimensions()[Unit::Current] == 1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(1e-9));
}

TEST_CASE("Unit parse picoampere pA", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("pA"));
    REQUIRE(u.dimensions()[Unit::Current] == 1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(1e-12));
}

TEST_CASE("Unit parse kilohertz kHz", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("kHz"));
    REQUIRE(u.dimensions()[Unit::Time] == -1);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(1e3));
}

// ===== Compound units =====

TEST_CASE("Unit parse m/s^2", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("m/s^2"));
    // meter / second^2 → length=1, time=-2
    REQUIRE(u.dimensions()[Unit::Length] == 1);
    REQUIRE(u.dimensions()[Unit::Time] == -2);
    REQUIRE_THAT(u.scaleToSI(), WithinRel(1.0));
}

TEST_CASE("Unit parse kg*m*s^-2 equals Newton dimensions", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("kg*m/s^2"));
    auto newton = Unit::parse(QStringLiteral("N"));
    REQUIRE(u.isCompatible(newton));
}

// ===== Edge cases =====

TEST_CASE("Unit parse empty string is dimensionless", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral(""));
    REQUIRE(u == Unit::dimensionless());
}

TEST_CASE("Unit parse invalid string throws", "[unit][parse]") {
    REQUIRE_THROWS_AS(Unit::parse(QStringLiteral("xyz")), std::invalid_argument);
}

TEST_CASE("Unit parse preserves display symbol", "[unit][parse]") {
    auto u = Unit::parse(QStringLiteral("mV"));
    REQUIRE(u.symbol() == QStringLiteral("mV"));
}
