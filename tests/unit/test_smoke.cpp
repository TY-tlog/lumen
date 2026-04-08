// Phase 0 smoke tests: verify the test harness and Qt link work.

#include <catch2/catch_test_macros.hpp>

#include <QString>

TEST_CASE("Catch2 harness works", "[smoke]") {
    REQUIRE(1 + 1 == 2);
}

TEST_CASE("Qt is linked and usable", "[smoke]") {
    QString s = "Lumen";
    REQUIRE(s.length() == 5);
    REQUIRE(s.toUpper() == "LUMEN");
}
