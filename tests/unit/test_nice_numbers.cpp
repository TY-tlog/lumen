#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "plot/NiceNumbers.h"

#include <cmath>

using lumen::plot::NiceNumbers;
using lumen::plot::TickResult;
using Catch::Matchers::WithinAbs;

TEST_CASE("NiceNumbers: [0, 700] produces nice ticks covering the range",
          "[plot][nice_numbers]")
{
    const auto result = NiceNumbers::compute(0.0, 700.0);

    REQUIRE(!result.values.empty());
    REQUIRE(result.spacing > 0.0);

    // Ticks must cover the data range.
    CHECK(result.values.front() <= 0.0);
    CHECK(result.values.back() >= 700.0);

    // Spacing should be a "nice" 1-2-5 pattern value (100 or 200).
    const bool isNice = (std::abs(result.spacing - 100.0) < 1e-10) ||
                        (std::abs(result.spacing - 200.0) < 1e-10);
    CHECK(isNice);

    // Ticks should be evenly spaced and multiples of spacing.
    for (double tick : result.values) {
        const double remainder = std::fmod(std::abs(tick) + 1e-14, result.spacing);
        const double roundedRemainder = std::min(remainder, result.spacing - remainder);
        CHECK_THAT(roundedRemainder, WithinAbs(0.0, 1e-8));
    }
}

TEST_CASE("NiceNumbers: [0, 700] with more ticks gives spacing 100",
          "[plot][nice_numbers]")
{
    // With a higher target tick count, spacing should be 100.
    const auto result = NiceNumbers::compute(0.0, 700.0, 10);

    REQUIRE(!result.values.empty());
    CHECK_THAT(result.spacing, WithinAbs(100.0, 1e-10));
    CHECK(result.values.front() <= 0.0);
    CHECK(result.values.back() >= 700.0);
}

TEST_CASE("NiceNumbers: [-80, 40] produces nice ticks",
          "[plot][nice_numbers]")
{
    const auto result = NiceNumbers::compute(-80.0, 40.0);

    REQUIRE(!result.values.empty());
    CHECK(result.spacing > 0.0);

    // Range is 120, with ~7 ticks, spacing should be 20.
    CHECK_THAT(result.spacing, WithinAbs(20.0, 1e-10));
    CHECK(result.values.front() <= -80.0);
    CHECK(result.values.back() >= 40.0);
}

TEST_CASE("NiceNumbers: [-38.7, -37.9] produces fractional ticks",
          "[plot][nice_numbers]")
{
    const auto result = NiceNumbers::compute(-38.7, -37.9);

    REQUIRE(!result.values.empty());
    CHECK(result.spacing > 0.0);
    CHECK(result.decimalPlaces >= 1);
    CHECK(result.values.front() <= -38.7);
    CHECK(result.values.back() >= -37.9);

    // Should have multiple ticks within the range.
    CHECK(result.values.size() >= 3);
}

TEST_CASE("NiceNumbers: zero range expands", "[plot][nice_numbers]")
{
    const auto result = NiceNumbers::compute(5.0, 5.0);

    REQUIRE(!result.values.empty());
    CHECK(result.spacing > 0.0);
    // The range should have been expanded around 5.0.
    CHECK(result.values.front() <= 5.0);
    CHECK(result.values.back() >= 5.0);
}

TEST_CASE("NiceNumbers: single point at zero", "[plot][nice_numbers]")
{
    const auto result = NiceNumbers::compute(0.0, 0.0);

    REQUIRE(!result.values.empty());
    CHECK(result.spacing > 0.0);
}

TEST_CASE("NiceNumbers: negative range (min > max) handled",
          "[plot][nice_numbers]")
{
    // If min > max, should still produce valid ticks.
    const auto result = NiceNumbers::compute(100.0, 0.0);

    REQUIRE(!result.values.empty());
    CHECK(result.spacing > 0.0);
    CHECK(result.values.front() <= 0.0);
    CHECK(result.values.back() >= 100.0);
}

TEST_CASE("NiceNumbers: very small range", "[plot][nice_numbers]")
{
    const auto result = NiceNumbers::compute(1.0000, 1.0005);

    REQUIRE(!result.values.empty());
    CHECK(result.spacing > 0.0);
    CHECK(result.decimalPlaces >= 3);
}

TEST_CASE("NiceNumbers: large range", "[plot][nice_numbers]")
{
    const auto result = NiceNumbers::compute(0.0, 1e7);

    REQUIRE(!result.values.empty());
    CHECK(result.spacing > 0.0);
    CHECK(result.values.front() <= 0.0);
    CHECK(result.values.back() >= 1e7);
}
