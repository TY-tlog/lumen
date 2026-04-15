#include <catch2/catch_test_macros.hpp>

#include <core/commands/StyleEditCommand.h>
#include <style/theme_fork.h>
#include <style/theme_registry.h>

using namespace lumen::style;

TEST_CASE("ThemeFork: fork builtin creates user theme", "[theme_fork]") {
    ThemeRegistry reg;
    CHECK(forkTheme(reg, "lumen-light", "my-light"));
    CHECK(reg.hasTheme("my-light"));
    CHECK_FALSE(reg.isBuiltin("my-light"));
    CHECK(reg.activeTheme() == "my-light");
}

TEST_CASE("ThemeFork: fork preserves source style", "[theme_fork]") {
    ThemeRegistry reg;
    forkTheme(reg, "lumen-light", "fork1");
    Style original = reg.theme("lumen-light");
    Style forked = reg.theme("fork1");
    CHECK(original == forked);
}

TEST_CASE("ThemeFork: cannot fork to builtin name", "[theme_fork]") {
    ThemeRegistry reg;
    CHECK_FALSE(forkTheme(reg, "lumen-light", "lumen-dark"));
}

TEST_CASE("ThemeFork: fork nonexistent source fails", "[theme_fork]") {
    ThemeRegistry reg;
    CHECK_FALSE(forkTheme(reg, "nonexistent", "new"));
}

TEST_CASE("StyleEditCommand: description includes level", "[style_edit]") {
    // Verify command description formatting.
    lumen::core::commands::StyleEditCommand cmd(
        CascadeLevel::ElementOverride,
        "stroke.color", {}, {},
        nullptr, nullptr, "lumen-light");
    CHECK(cmd.description().contains("element"));
}

TEST_CASE("StyleEditCommand: theme level description", "[style_edit]") {
    lumen::core::commands::StyleEditCommand cmd(
        CascadeLevel::Theme,
        "fill.color", {}, {},
        nullptr, nullptr, "publication");
    CHECK(cmd.description().contains("publication"));
}
