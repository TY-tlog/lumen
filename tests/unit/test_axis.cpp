#include <catch2/catch_test_macros.hpp>

#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <memory>
#include <plot/Axis.h>
#include <plot/LineSeries.h>
#include <plot/PlotStyle.h>

#include <cmath>
#include <vector>

using namespace lumen::plot;
using namespace lumen::data;

TEST_CASE("Axis manual range produces ticks", "[axis]") {
    Axis axis(AxisOrientation::Horizontal);
    axis.setRange(0.0, 700.0);

    auto marks = axis.ticks();
    REQUIRE(!marks.empty());
    // NiceNumbers may extend slightly beyond data range for "nice" values.
    REQUIRE(marks.front().value >= -100.0);
    REQUIRE(marks.back().value <= 800.0);
}

TEST_CASE("Axis label is stored", "[axis]") {
    Axis axis(AxisOrientation::Vertical);
    axis.setLabel("voltage_mV");
    REQUIRE(axis.label() == "voltage_mV");
}

TEST_CASE("Axis orientation is stored", "[axis]") {
    Axis hAxis(AxisOrientation::Horizontal);
    Axis vAxis(AxisOrientation::Vertical);
    REQUIRE(hAxis.orientation() == AxisOrientation::Horizontal);
    REQUIRE(vAxis.orientation() == AxisOrientation::Vertical);
}

TEST_CASE("Axis autoRange from one series", "[axis]") {
    std::vector<double> xData = {0.0, 1.0, 2.0, 3.0};
    std::vector<double> yData = {10.0, 20.0, 15.0, 25.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), xData);
    auto yCol = std::make_shared<lumen::data::Rank1Dataset>("y", lumen::data::Unit::dimensionless(), yData);

    LineSeries series(xCol, yCol, PlotStyle::fromPalette(0));

    Axis xAxis(AxisOrientation::Horizontal);
    xAxis.autoRange({series});

    // Should cover [0, 3] with 5% padding.
    REQUIRE(xAxis.min() < 0.0);
    REQUIRE(xAxis.max() > 3.0);

    Axis yAxis(AxisOrientation::Vertical);
    yAxis.autoRange({series});
    REQUIRE(yAxis.min() < 10.0);
    REQUIRE(yAxis.max() > 25.0);
}

TEST_CASE("Axis autoRange from multiple series takes union", "[axis]") {
    std::vector<double> x1 = {0.0, 1.0};
    std::vector<double> y1 = {10.0, 20.0};
    std::vector<double> x2 = {2.0, 3.0};
    std::vector<double> y2 = {5.0, 30.0};
    auto xCol1 = std::make_shared<lumen::data::Rank1Dataset>("x1", lumen::data::Unit::dimensionless(), x1);
    auto yCol1 = std::make_shared<lumen::data::Rank1Dataset>("y1", lumen::data::Unit::dimensionless(), y1);
    auto xCol2 = std::make_shared<lumen::data::Rank1Dataset>("x2", lumen::data::Unit::dimensionless(), x2);
    auto yCol2 = std::make_shared<lumen::data::Rank1Dataset>("y2", lumen::data::Unit::dimensionless(), y2);

    LineSeries s1(xCol1, yCol1, PlotStyle::fromPalette(0));
    LineSeries s2(xCol2, yCol2, PlotStyle::fromPalette(1));

    Axis xAxis(AxisOrientation::Horizontal);
    xAxis.autoRange({s1, s2});
    REQUIRE(xAxis.min() < 0.0);
    REQUIRE(xAxis.max() > 3.0);

    Axis yAxis(AxisOrientation::Vertical);
    yAxis.autoRange({s1, s2});
    REQUIRE(yAxis.min() < 5.0);
    REQUIRE(yAxis.max() > 30.0);
}

TEST_CASE("Axis autoRange empty series list", "[axis]") {
    Axis axis(AxisOrientation::Horizontal);
    axis.autoRange({});
    REQUIRE(axis.min() == 0.0);
    REQUIRE(axis.max() == 1.0);
}

TEST_CASE("Axis ticks for fractional range", "[axis]") {
    Axis axis(AxisOrientation::Vertical);
    axis.setRange(-38.7, -37.9);

    auto marks = axis.ticks();
    REQUIRE(!marks.empty());
    // All tick labels should have decimal places.
    for (const auto& m : marks) {
        REQUIRE(m.label.contains('.'));
    }
}

// Phase 2.5 additional tests

TEST_CASE("Axis autoRange adds 5% padding", "[axis][p2.5]") {
    std::vector<double> x = {0.0, 100.0};
    std::vector<double> y = {0.0, 100.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), x);
    auto yCol = std::make_shared<lumen::data::Rank1Dataset>("y", lumen::data::Unit::dimensionless(), y);
    lumen::plot::LineSeries series(xCol, yCol, lumen::plot::PlotStyle::fromPalette(0));

    lumen::plot::Axis axis(lumen::plot::AxisOrientation::Horizontal);
    axis.autoRange({series});
    CHECK(axis.min() < 0.0);
    CHECK(axis.max() > 100.0);
    // 5% of 100 = 5
    CHECK(axis.min() >= -10.0);
    CHECK(axis.max() <= 110.0);
}

TEST_CASE("Axis manual setRange overrides autoRange", "[axis][p2.5]") {
    std::vector<double> x = {0.0, 100.0};
    std::vector<double> y = {0.0, 100.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), x);
    auto yCol = std::make_shared<lumen::data::Rank1Dataset>("y", lumen::data::Unit::dimensionless(), y);
    lumen::plot::LineSeries series(xCol, yCol, lumen::plot::PlotStyle::fromPalette(0));

    lumen::plot::Axis axis(lumen::plot::AxisOrientation::Horizontal);
    axis.autoRange({series});
    axis.setRange(10.0, 20.0);
    CHECK(axis.min() == 10.0);
    CHECK(axis.max() == 20.0);
}

TEST_CASE("Axis ticks from two series union", "[axis][p2.5]") {
    std::vector<double> x1 = {0.0, 50.0};
    std::vector<double> y1 = {0.0, 50.0};
    std::vector<double> x2 = {60.0, 200.0};
    std::vector<double> y2 = {60.0, 200.0};
    auto xCol1 = std::make_shared<lumen::data::Rank1Dataset>("x1", lumen::data::Unit::dimensionless(), x1);
    auto yCol1 = std::make_shared<lumen::data::Rank1Dataset>("y1", lumen::data::Unit::dimensionless(), y1);
    auto xCol2 = std::make_shared<lumen::data::Rank1Dataset>("x2", lumen::data::Unit::dimensionless(), x2);
    auto yCol2 = std::make_shared<lumen::data::Rank1Dataset>("y2", lumen::data::Unit::dimensionless(), y2);
    lumen::plot::LineSeries s1(xCol1, yCol1, lumen::plot::PlotStyle::fromPalette(0));
    lumen::plot::LineSeries s2(xCol2, yCol2, lumen::plot::PlotStyle::fromPalette(1));

    lumen::plot::Axis axis(lumen::plot::AxisOrientation::Horizontal);
    axis.autoRange({s1, s2});
    CHECK(axis.min() < 0.0);
    CHECK(axis.max() > 200.0);
    auto ticks = axis.ticks();
    CHECK(!ticks.empty());
}
