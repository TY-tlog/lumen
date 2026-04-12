// Unit tests for ReactiveMode enum.

#include <catch2/catch_test_macros.hpp>

#include <core/reactive/ReactiveMode.h>

using namespace lumen::reactive;

TEST_CASE("ReactiveMode -- enum values are distinct", "[reactive_mode]") {
    REQUIRE(Mode::Static != Mode::DAG);
    REQUIRE(Mode::DAG != Mode::Bidirectional);
    REQUIRE(Mode::Static != Mode::Bidirectional);
}

TEST_CASE("ReactiveMode -- Static is the zero value", "[reactive_mode]") {
    REQUIRE(static_cast<int>(Mode::Static) == 0);
}

TEST_CASE("ReactiveMode -- DAG is value 1", "[reactive_mode]") {
    REQUIRE(static_cast<int>(Mode::DAG) == 1);
}

TEST_CASE("ReactiveMode -- Bidirectional is value 2", "[reactive_mode]") {
    REQUIRE(static_cast<int>(Mode::Bidirectional) == 2);
}

TEST_CASE("ReactiveMode -- round-trip through int cast", "[reactive_mode]") {
    for (int i = 0; i < 3; ++i) {
        auto m = static_cast<Mode>(i);
        REQUIRE(static_cast<int>(m) == i);
    }
}
