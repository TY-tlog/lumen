// Unit tests for Unit arithmetic operators.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Unit.h>

using namespace lumen::data;
using Catch::Matchers::WithinRel;

TEST_CASE("Unit multiply adds dimension exponents", "[unit][arithmetic]") {
    // meter * meter = m^2
    auto m = Unit::parse(QStringLiteral("m"));
    auto m2 = m * m;
    REQUIRE(m2.dimensions()[Unit::Length] == 2);
    REQUIRE_THAT(m2.scaleToSI(), WithinRel(1.0));
}

TEST_CASE("Unit divide subtracts dimension exponents", "[unit][arithmetic]") {
    // meter / second = m/s
    auto m = Unit::parse(QStringLiteral("m"));
    auto s = Unit::parse(QStringLiteral("s"));
    auto ms = m / s;
    REQUIRE(ms.dimensions()[Unit::Length] == 1);
    REQUIRE(ms.dimensions()[Unit::Time] == -1);
}

TEST_CASE("Unit pow multiplies exponents", "[unit][arithmetic]") {
    auto m = Unit::parse(QStringLiteral("m"));
    auto m3 = m.pow(3);
    REQUIRE(m3.dimensions()[Unit::Length] == 3);
    REQUIRE_THAT(m3.scaleToSI(), WithinRel(1.0));
}

TEST_CASE("Unit pow zero gives dimensionless", "[unit][arithmetic]") {
    auto m = Unit::parse(QStringLiteral("m"));
    auto m0 = m.pow(0);
    REQUIRE(m0.dimensions()[Unit::Length] == 0);
    REQUIRE_THAT(m0.scaleToSI(), WithinRel(1.0));
}

TEST_CASE("Unit multiply scales multiply", "[unit][arithmetic]") {
    // mV * mA: scales multiply to 1e-6
    auto mV = Unit::parse(QStringLiteral("mV"));
    auto nA = Unit::parse(QStringLiteral("nA"));
    auto product = mV * nA;
    REQUIRE_THAT(product.scaleToSI(), WithinRel(1e-12));
}

TEST_CASE("Unit V = I * R dimensional check", "[unit][arithmetic]") {
    auto ampere = Unit::parse(QStringLiteral("A"));
    auto ohm = Unit::parse(QStringLiteral("Ohm"));
    auto volt = Unit::parse(QStringLiteral("V"));

    auto computed = ampere * ohm;
    REQUIRE(computed.isCompatible(volt));
}

TEST_CASE("Unit Newton dimensional check: kg*m/s^2", "[unit][arithmetic]") {
    auto kg = Unit::parse(QStringLiteral("kg"));
    auto m = Unit::parse(QStringLiteral("m"));
    auto s = Unit::parse(QStringLiteral("s"));
    auto newton = Unit::parse(QStringLiteral("N"));

    auto computed = (kg * m) / s.pow(2);
    REQUIRE(computed.isCompatible(newton));
}

TEST_CASE("Unit Watt dimensional check: J/s", "[unit][arithmetic]") {
    auto joule = Unit::parse(QStringLiteral("J"));
    auto second = Unit::parse(QStringLiteral("s"));
    auto watt = Unit::parse(QStringLiteral("W"));

    auto computed = joule / second;
    REQUIRE(computed.isCompatible(watt));
}
