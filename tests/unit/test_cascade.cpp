#include <catch2/catch_test_macros.hpp>

#include <style/cascade.h>
#include <style/types.h>

using namespace lumen::style;

TEST_CASE("Cascade: element override has highest priority", "[cascade]") {
    Style theme, preset, plot, element;
    theme.backgroundColor = QColor(Qt::white);
    preset.backgroundColor = QColor(Qt::gray);
    plot.backgroundColor = QColor(Qt::blue);
    element.backgroundColor = QColor(Qt::red);

    Style resolved = cascade(theme, preset, plot, element);
    REQUIRE(resolved.backgroundColor.has_value());
    CHECK(*resolved.backgroundColor == QColor(Qt::red));
}

TEST_CASE("Cascade: nullopt falls through to lower level", "[cascade]") {
    Style theme, preset, plot, element;
    theme.backgroundColor = QColor(Qt::white);
    // preset, plot, element leave backgroundColor as nullopt.

    Style resolved = cascade(theme, preset, plot, element);
    REQUIRE(resolved.backgroundColor.has_value());
    CHECK(*resolved.backgroundColor == QColor(Qt::white));
}

TEST_CASE("Cascade: plot overrides preset", "[cascade]") {
    Style theme, preset, plot, element;
    preset.lineWidth = 1.0;
    plot.lineWidth = 2.5;

    Style resolved = cascade(theme, preset, plot, element);
    REQUIRE(resolved.lineWidth.has_value());
    CHECK(*resolved.lineWidth == 2.5);
}

TEST_CASE("Cascade: preset overrides theme", "[cascade]") {
    Style theme, preset, plot, element;
    theme.lineWidth = 1.0;
    preset.lineWidth = 1.5;

    Style resolved = cascade(theme, preset, plot, element);
    REQUIRE(resolved.lineWidth.has_value());
    CHECK(*resolved.lineWidth == 1.5);
}

TEST_CASE("Cascade: all nullopt produces empty Style", "[cascade]") {
    Style theme, preset, plot, element;
    Style resolved = cascade(theme, preset, plot, element);
    CHECK_FALSE(resolved.backgroundColor.has_value());
    CHECK_FALSE(resolved.lineWidth.has_value());
    CHECK_FALSE(resolved.stroke.has_value());
}

TEST_CASE("Cascade: sub-style merge (stroke)", "[cascade]") {
    Style theme, preset, plot, element;
    theme.stroke = StrokeStyle{QColor(Qt::black), 1.0, std::nullopt, std::nullopt, std::nullopt};
    element.stroke = StrokeStyle{std::nullopt, 2.0, std::nullopt, std::nullopt, std::nullopt};

    Style resolved = cascade(theme, preset, plot, element);
    REQUIRE(resolved.stroke.has_value());
    CHECK(*resolved.stroke->color == QColor(Qt::black));  // from theme
    CHECK(*resolved.stroke->width == 2.0);                 // from element
}

TEST_CASE("Cascade: mixed sources across properties", "[cascade]") {
    Style theme, preset, plot, element;
    theme.backgroundColor = QColor(Qt::white);
    theme.text = TextStyle{QString("Inter"), 10.0, std::nullopt, std::nullopt, std::nullopt};
    preset.lineWidth = 1.5;
    element.markerSize = 6.0;

    Style resolved = cascade(theme, preset, plot, element);
    CHECK(*resolved.backgroundColor == QColor(Qt::white));
    CHECK(*resolved.text->family == QString("Inter"));
    CHECK(*resolved.lineWidth == 1.5);
    CHECK(*resolved.markerSize == 6.0);
}

TEST_CASE("Cascade: full override at element level", "[cascade]") {
    Style theme, preset, plot, element;
    theme.lineWidth = 1.0;
    theme.markerSize = 4.0;
    element.lineWidth = 3.0;
    element.markerSize = 8.0;

    Style resolved = cascade(theme, preset, plot, element);
    CHECK(*resolved.lineWidth == 3.0);
    CHECK(*resolved.markerSize == 8.0);
}

TEST_CASE("CascadeTrace: records correct source levels", "[cascade]") {
    Style theme, preset, plot, element;
    theme.backgroundColor = QColor(Qt::white);
    preset.lineWidth = 1.5;
    element.foregroundColor = QColor(Qt::black);

    CascadeTrace trace;
    cascadeWithTrace(theme, "lumen-light", preset, "scatter",
                     plot, element, trace);

    bool foundBg = false, foundLw = false, foundFg = false;
    for (const auto& e : trace) {
        if (e.property == "backgroundColor") {
            CHECK(e.source == CascadeLevel::Theme);
            foundBg = true;
        }
        if (e.property == "lineWidth") {
            CHECK(e.source == CascadeLevel::Preset);
            foundLw = true;
        }
        if (e.property == "foregroundColor") {
            CHECK(e.source == CascadeLevel::ElementOverride);
            foundFg = true;
        }
    }
    CHECK(foundBg);
    CHECK(foundLw);
    CHECK(foundFg);
}

TEST_CASE("CascadeTrace: empty cascade produces empty trace", "[cascade]") {
    Style theme, preset, plot, element;
    CascadeTrace trace;
    cascadeWithTrace(theme, "t", preset, "p", plot, element, trace);
    CHECK(trace.isEmpty());
}

TEST_CASE("Cascade: colormapName and contourLevels", "[cascade]") {
    Style theme, preset, plot, element;
    theme.colormapName = QString("viridis");
    plot.contourLevels = 10;

    Style resolved = cascade(theme, preset, plot, element);
    CHECK(*resolved.colormapName == QString("viridis"));
    CHECK(*resolved.contourLevels == 10);
}

TEST_CASE("Cascade: barWidth from preset", "[cascade]") {
    Style theme, preset, plot, element;
    preset.barWidth = 0.8;

    Style resolved = cascade(theme, preset, plot, element);
    CHECK(*resolved.barWidth == 0.8);
}
