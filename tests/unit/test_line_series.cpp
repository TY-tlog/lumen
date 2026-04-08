#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "data/Column.h"
#include "plot/LineSeries.h"
#include "plot/PlotStyle.h"

#include <cmath>
#include <limits>
#include <vector>

using lumen::data::Column;
using lumen::plot::LineSeries;
using lumen::plot::PlotStyle;
using Catch::Matchers::WithinAbs;

namespace {

const auto kNaN = std::numeric_limits<double>::quiet_NaN();

} // namespace

TEST_CASE("LineSeries: no NaN produces single polyline",
          "[plot][line_series]")
{
    Column xCol("x", std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0});
    Column yCol("y", std::vector<double>{10.0, 20.0, 30.0, 40.0, 50.0});

    const LineSeries series(&xCol, &yCol, PlotStyle::fromPalette(0), "test");
    const auto polylines = series.buildPolylines();

    REQUIRE(polylines.size() == 1);
    CHECK(polylines[0].size() == 5);
    CHECK_THAT(polylines[0][0].x(), WithinAbs(1.0, 1e-10));
    CHECK_THAT(polylines[0][0].y(), WithinAbs(10.0, 1e-10));
    CHECK_THAT(polylines[0][4].x(), WithinAbs(5.0, 1e-10));
    CHECK_THAT(polylines[0][4].y(), WithinAbs(50.0, 1e-10));
}

TEST_CASE("LineSeries: NaN in middle splits into two polylines",
          "[plot][line_series]")
{
    Column xCol("x", std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0});
    Column yCol("y", std::vector<double>{10.0, 20.0, kNaN, 40.0, 50.0});

    const LineSeries series(&xCol, &yCol, PlotStyle::fromPalette(0));
    const auto polylines = series.buildPolylines();

    REQUIRE(polylines.size() == 2);
    CHECK(polylines[0].size() == 2); // points 0, 1
    CHECK(polylines[1].size() == 2); // points 3, 4
}

TEST_CASE("LineSeries: NaN at start skips first segment",
          "[plot][line_series]")
{
    Column xCol("x", std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0});
    Column yCol("y", std::vector<double>{kNaN, kNaN, 30.0, 40.0, 50.0});

    const LineSeries series(&xCol, &yCol, PlotStyle::fromPalette(0));
    const auto polylines = series.buildPolylines();

    REQUIRE(polylines.size() == 1);
    CHECK(polylines[0].size() == 3); // points 2, 3, 4
}

TEST_CASE("LineSeries: all NaN produces empty polylines",
          "[plot][line_series]")
{
    Column xCol("x", std::vector<double>{kNaN, kNaN, kNaN});
    Column yCol("y", std::vector<double>{kNaN, kNaN, kNaN});

    const LineSeries series(&xCol, &yCol, PlotStyle::fromPalette(0));
    const auto polylines = series.buildPolylines();

    CHECK(polylines.empty());
}

TEST_CASE("LineSeries: NaN in X column also breaks the line",
          "[plot][line_series]")
{
    Column xCol("x", std::vector<double>{1.0, kNaN, 3.0, 4.0});
    Column yCol("y", std::vector<double>{10.0, 20.0, 30.0, 40.0});

    const LineSeries series(&xCol, &yCol, PlotStyle::fromPalette(0));
    const auto polylines = series.buildPolylines();

    REQUIRE(polylines.size() == 2);
    CHECK(polylines[0].size() == 1); // point 0
    CHECK(polylines[1].size() == 2); // points 2, 3
}

TEST_CASE("LineSeries: dataRange ignores NaN", "[plot][line_series]")
{
    Column xCol("x", std::vector<double>{1.0, kNaN, 3.0, 5.0});
    Column yCol("y", std::vector<double>{10.0, 20.0, kNaN, 50.0});

    const LineSeries series(&xCol, &yCol, PlotStyle::fromPalette(0));
    const auto range = series.dataRange();

    // Only points where both X and Y are non-NaN: (1, 10) and (5, 50).
    CHECK_THAT(range.xMin, WithinAbs(1.0, 1e-10));
    CHECK_THAT(range.xMax, WithinAbs(5.0, 1e-10));
    CHECK_THAT(range.yMin, WithinAbs(10.0, 1e-10));
    CHECK_THAT(range.yMax, WithinAbs(50.0, 1e-10));
}

TEST_CASE("LineSeries: dataRange with all NaN returns zeros",
          "[plot][line_series]")
{
    Column xCol("x", std::vector<double>{kNaN, kNaN});
    Column yCol("y", std::vector<double>{kNaN, kNaN});

    const LineSeries series(&xCol, &yCol, PlotStyle::fromPalette(0));
    const auto range = series.dataRange();

    CHECK_THAT(range.xMin, WithinAbs(0.0, 1e-10));
    CHECK_THAT(range.xMax, WithinAbs(0.0, 1e-10));
    CHECK_THAT(range.yMin, WithinAbs(0.0, 1e-10));
    CHECK_THAT(range.yMax, WithinAbs(0.0, 1e-10));
}

TEST_CASE("LineSeries: style and name accessors", "[plot][line_series]")
{
    Column xCol("x", std::vector<double>{1.0, 2.0});
    Column yCol("y", std::vector<double>{3.0, 4.0});

    const auto style = PlotStyle::fromPalette(2);
    const LineSeries series(&xCol, &yCol, style, "my series");

    CHECK(series.name() == "my series");
    CHECK(series.style().color == style.color);
    CHECK_THAT(series.style().lineWidth, WithinAbs(style.lineWidth, 1e-10));
}

TEST_CASE("LineSeries: throws on non-Double columns",
          "[plot][line_series]")
{
    Column xCol("x", std::vector<int64_t>{1, 2, 3});
    Column yCol("y", std::vector<double>{1.0, 2.0, 3.0});

    CHECK_THROWS_AS(
        LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0)),
        std::invalid_argument);
}

TEST_CASE("LineSeries: throws on mismatched row counts",
          "[plot][line_series]")
{
    Column xCol("x", std::vector<double>{1.0, 2.0, 3.0});
    Column yCol("y", std::vector<double>{1.0, 2.0});

    CHECK_THROWS_AS(
        LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0)),
        std::invalid_argument);
}

TEST_CASE("PlotStyle: fromPalette wraps around", "[plot][plot_style]")
{
    const auto s0 = PlotStyle::fromPalette(0);
    const auto s8 = PlotStyle::fromPalette(8);

    // Index 8 should wrap to index 0.
    CHECK(s0.color == s8.color);
}

// Phase 2.5 additional tests

TEST_CASE("LineSeries single data point", "[lineseries][p2.5]") {
    std::vector<double> x = {1.0};
    std::vector<double> y = {2.0};
    lumen::data::Column xCol("x", x);
    lumen::data::Column yCol("y", y);
    lumen::plot::LineSeries series(&xCol, &yCol, lumen::plot::PlotStyle::fromPalette(0));
    auto polys = series.buildPolylines();
    REQUIRE(polys.size() == 1);
    CHECK(polys[0].size() == 1);
}

TEST_CASE("LineSeries multiple NaN gaps", "[lineseries][p2.5]") {
    std::vector<double> x = {1, 2, 3, 4, 5};
    double nan = std::numeric_limits<double>::quiet_NaN();
    std::vector<double> y = {1, nan, 3, nan, 5};
    lumen::data::Column xCol("x", x);
    lumen::data::Column yCol("y", y);
    lumen::plot::LineSeries series(&xCol, &yCol, lumen::plot::PlotStyle::fromPalette(0));
    auto polys = series.buildPolylines();
    CHECK(polys.size() == 3);
}

TEST_CASE("LineSeries all NaN no crash", "[lineseries][p2.5]") {
    double nan = std::numeric_limits<double>::quiet_NaN();
    std::vector<double> x = {nan, nan, nan};
    std::vector<double> y = {nan, nan, nan};
    lumen::data::Column xCol("x", x);
    lumen::data::Column yCol("y", y);
    lumen::plot::LineSeries series(&xCol, &yCol, lumen::plot::PlotStyle::fromPalette(0));
    auto polys = series.buildPolylines();
    CHECK(polys.empty());
}
