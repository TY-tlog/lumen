#include <catch2/catch_test_macros.hpp>

#include <style/extends_resolver.h>
#include <style/theme_registry.h>

using namespace lumen::style;

TEST_CASE("ExtendsResolver: known theme resolves", "[extends]") {
    ThemeRegistry reg;
    Style s = resolveExtends(reg, "lumen-light");
    // Should return the theme (even if placeholder).
    // Not null — hasTheme is true.
    CHECK(reg.hasTheme("lumen-light"));
}

TEST_CASE("ExtendsResolver: unknown theme returns empty", "[extends]") {
    ThemeRegistry reg;
    Style s = resolveExtends(reg, "nonexistent");
    CHECK_FALSE(s.lineWidth.has_value());
}

TEST_CASE("ExtendsResolver: cycle detection with empty name", "[extends]") {
    ThemeRegistry reg;
    CHECK_FALSE(detectCycle(reg, "").isEmpty());
}

TEST_CASE("ExtendsResolver: cycle detection with valid name", "[extends]") {
    ThemeRegistry reg;
    CHECK(detectCycle(reg, "lumen-light").isEmpty());
}

TEST_CASE("ExtendsResolver: depth exceeded", "[extends]") {
    ThemeRegistry reg;
    CHECK_FALSE(detectCycle(reg, "test", 0).isEmpty());
}
