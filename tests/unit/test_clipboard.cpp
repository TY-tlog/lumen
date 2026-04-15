#include <catch2/catch_test_macros.hpp>

#include <style/clipboard.h>

using namespace lumen::style;

TEST_CASE("Clipboard: initially empty", "[clipboard]") {
    StyleClipboard clip;
    CHECK_FALSE(clip.hasContent());
}

TEST_CASE("Clipboard: copy sets content", "[clipboard]") {
    StyleClipboard clip;
    Style s;
    s.lineWidth = 2.0;
    clip.copy(s, "series.line");
    CHECK(clip.hasContent());
    CHECK(clip.sourceElement() == "series.line");
}

TEST_CASE("Clipboard: paste same type returns full style", "[clipboard]") {
    StyleClipboard clip;
    Style s;
    s.lineWidth = 2.0;
    s.stroke = StrokeStyle{QColor(Qt::red), 1.5, std::nullopt, std::nullopt, std::nullopt};
    clip.copy(s, "series.line");

    int dropped = 0;
    Style pasted = clip.paste("series.marker", dropped);
    CHECK(dropped == 0);
    REQUIRE(pasted.lineWidth.has_value());
    CHECK(*pasted.lineWidth == 2.0);
}

TEST_CASE("Clipboard: cross-type paste drops marker for axis", "[clipboard]") {
    StyleClipboard clip;
    Style s;
    s.stroke = StrokeStyle{QColor(Qt::blue), 1.0, std::nullopt, std::nullopt, std::nullopt};
    s.marker = MarkerStyle{MarkerShape::Circle, 5.0, std::nullopt, std::nullopt};
    clip.copy(s, "series.scatter");

    int dropped = 0;
    Style pasted = clip.paste("axis.spine", dropped);
    // Marker should be dropped (axis doesn't use markers).
    CHECK(dropped >= 1);
    CHECK(pasted.stroke.has_value());
    CHECK_FALSE(pasted.marker.has_value());
}

TEST_CASE("Clipboard: clear removes content", "[clipboard]") {
    StyleClipboard clip;
    Style s;
    s.lineWidth = 1.0;
    clip.copy(s, "test");
    clip.clear();
    CHECK_FALSE(clip.hasContent());
}

TEST_CASE("Clipboard: paste empty returns empty", "[clipboard]") {
    StyleClipboard clip;
    int dropped = 0;
    Style pasted = clip.paste("any", dropped);
    CHECK_FALSE(pasted.lineWidth.has_value());
}
