// Unit tests for DesignTokens — verify values match design-system.md.

#include <catch2/catch_test_macros.hpp>

#include <style/DesignTokens.h>

#include <QColor>

using namespace lumen::tokens;

TEST_CASE("Color tokens match design-system.md light palette", "[tokens][color]") {
    SECTION("background colors") {
        REQUIRE(color::background::primary == QColor(0xFF, 0xFF, 0xFF));
        REQUIRE(color::background::secondary == QColor(0xF5, 0xF5, 0xF7));
    }

    SECTION("surface colors") {
        REQUIRE(color::surface::elevated == QColor(0xFF, 0xFF, 0xFF));
        REQUIRE(color::surface::sunken == QColor(0xEC, 0xEC, 0xEE));
    }

    SECTION("border colors") {
        REQUIRE(color::border::subtle == QColor(0xE5, 0xE5, 0xEA));
        REQUIRE(color::border::strong == QColor(0xD1, 0xD1, 0xD6));
    }

    SECTION("text colors") {
        REQUIRE(color::text::primary == QColor(0x1D, 0x1D, 0x1F));
        REQUIRE(color::text::secondary == QColor(0x63, 0x63, 0x66));
        REQUIRE(color::text::tertiary == QColor(0x8E, 0x8E, 0x93));
    }

    SECTION("accent colors") {
        REQUIRE(color::accent::primary == QColor(0x0A, 0x84, 0xFF));
        REQUIRE(color::accent::muted == QColor(0xE5, 0xF0, 0xFF));
    }

    SECTION("semantic colors") {
        REQUIRE(color::success == QColor(0x30, 0xD1, 0x58));
        REQUIRE(color::warning == QColor(0xFF, 0x9F, 0x0A));
        REQUIRE(color::error == QColor(0xFF, 0x3B, 0x30));
    }

    SECTION("plot palette has 8 entries") {
        REQUIRE(color::plotPalette.size() == 8);
        REQUIRE(color::plotPalette[0] == QColor(0x0A, 0x84, 0xFF));  // blue
        REQUIRE(color::plotPalette[1] == QColor(0xFF, 0x9F, 0x0A));  // orange
    }
}

TEST_CASE("Spacing tokens match design-system.md", "[tokens][spacing]") {
    REQUIRE(spacing::xxs == 4);
    REQUIRE(spacing::xs == 8);
    REQUIRE(spacing::sm == 12);
    REQUIRE(spacing::md == 16);
    REQUIRE(spacing::lg == 24);
    REQUIRE(spacing::xl == 32);
    REQUIRE(spacing::xxl == 48);
    REQUIRE(spacing::xxxl == 64);
    REQUIRE(spacing::containerPadding == 16);
    REQUIRE(spacing::sectionGap == 24);
}

TEST_CASE("Radius tokens match design-system.md", "[tokens][radius]") {
    REQUIRE(radius::xs == 4);
    REQUIRE(radius::sm == 8);
    REQUIRE(radius::md == 12);
    REQUIRE(radius::lg == 16);
}

TEST_CASE("Typography tokens match design-system.md", "[tokens][typography]") {
    REQUIRE(typography::caption.sizePx == 11);
    REQUIRE(typography::footnote.sizePx == 12);
    REQUIRE(typography::body.sizePx == 13);
    REQUIRE(typography::bodyStrong.sizePx == 13);
    REQUIRE(typography::subhead.sizePx == 15);
    REQUIRE(typography::title3.sizePx == 17);
    REQUIRE(typography::title2.sizePx == 22);
    REQUIRE(typography::title1.sizePx == 28);

    REQUIRE(typography::body.weight == QFont::Normal);
    REQUIRE(typography::bodyStrong.weight == QFont::Medium);
    REQUIRE(typography::title3.weight == QFont::DemiBold);
    REQUIRE(typography::title1.weight == QFont::Bold);

    REQUIRE(typography::caption.lineHeightPx == 14);
    REQUIRE(typography::body.lineHeightPx == 18);
    REQUIRE(typography::title1.lineHeightPx == 34);
}

TEST_CASE("Motion tokens match design-system.md", "[tokens][motion]") {
    REQUIRE(motion::durationFastMs == 120);
    REQUIRE(motion::durationNormalMs == 200);
    REQUIRE(motion::durationSlowMs == 320);
}

TEST_CASE("Shell tokens match design-system.md", "[tokens][shell]") {
    REQUIRE(shell::toolbarHeight == 44);
}
