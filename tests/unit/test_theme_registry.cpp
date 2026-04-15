#include <catch2/catch_test_macros.hpp>

#include <style/theme_registry.h>
#include <style/types.h>

using namespace lumen::style;

TEST_CASE("ThemeRegistry: 6 builtin themes registered", "[theme_registry]") {
    ThemeRegistry reg;
    CHECK(reg.builtinNames().size() == 6);
}

TEST_CASE("ThemeRegistry: builtin names are correct", "[theme_registry]") {
    ThemeRegistry reg;
    QStringList names = reg.builtinNames();
    CHECK(names.contains(QStringLiteral("lumen-light")));
    CHECK(names.contains(QStringLiteral("lumen-dark")));
    CHECK(names.contains(QStringLiteral("publication")));
    CHECK(names.contains(QStringLiteral("colorblind-safe")));
    CHECK(names.contains(QStringLiteral("presentation")));
    CHECK(names.contains(QStringLiteral("print-bw")));
}

TEST_CASE("ThemeRegistry: hasTheme for builtins", "[theme_registry]") {
    ThemeRegistry reg;
    CHECK(reg.hasTheme(QStringLiteral("lumen-light")));
    CHECK(reg.hasTheme(QStringLiteral("publication")));
    CHECK_FALSE(reg.hasTheme(QStringLiteral("nonexistent")));
}

TEST_CASE("ThemeRegistry: isBuiltin", "[theme_registry]") {
    ThemeRegistry reg;
    CHECK(reg.isBuiltin(QStringLiteral("lumen-light")));
    CHECK_FALSE(reg.isBuiltin(QStringLiteral("my-custom")));
}

TEST_CASE("ThemeRegistry: cannot register user theme with builtin name", "[theme_registry]") {
    ThemeRegistry reg;
    Style s;
    s.lineWidth = 5.0;
    CHECK_FALSE(reg.registerUserTheme(QStringLiteral("lumen-light"), s));
}

TEST_CASE("ThemeRegistry: register and retrieve user theme", "[theme_registry]") {
    ThemeRegistry reg;
    Style s;
    s.lineWidth = 3.0;
    CHECK(reg.registerUserTheme(QStringLiteral("my-theme"), s));
    CHECK(reg.hasTheme(QStringLiteral("my-theme")));
    Style loaded = reg.theme(QStringLiteral("my-theme"));
    REQUIRE(loaded.lineWidth.has_value());
    CHECK(*loaded.lineWidth == 3.0);
}

TEST_CASE("ThemeRegistry: remove user theme", "[theme_registry]") {
    ThemeRegistry reg;
    Style s;
    reg.registerUserTheme(QStringLiteral("tmp"), s);
    CHECK(reg.removeUserTheme(QStringLiteral("tmp")));
    CHECK_FALSE(reg.hasTheme(QStringLiteral("tmp")));
}

TEST_CASE("ThemeRegistry: cannot remove builtin", "[theme_registry]") {
    ThemeRegistry reg;
    CHECK_FALSE(reg.removeUserTheme(QStringLiteral("lumen-light")));
    CHECK(reg.hasTheme(QStringLiteral("lumen-light")));
}

TEST_CASE("ThemeRegistry: active theme default is lumen-light", "[theme_registry]") {
    ThemeRegistry reg;
    CHECK(reg.activeTheme() == QStringLiteral("lumen-light"));
}

TEST_CASE("ThemeRegistry: setActiveTheme", "[theme_registry]") {
    ThemeRegistry reg;
    reg.setActiveTheme(QStringLiteral("publication"));
    CHECK(reg.activeTheme() == QStringLiteral("publication"));
}

TEST_CASE("ThemeRegistry: themeNames includes builtins and user", "[theme_registry]") {
    ThemeRegistry reg;
    Style s;
    reg.registerUserTheme(QStringLiteral("custom"), s);
    QStringList names = reg.themeNames();
    CHECK(names.size() >= 7);
    CHECK(names.contains(QStringLiteral("custom")));
    CHECK(names.contains(QStringLiteral("lumen-light")));
}
