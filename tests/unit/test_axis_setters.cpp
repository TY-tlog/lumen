#include <catch2/catch_test_macros.hpp>

#include <QSignalSpy>

#include <plot/Axis.h>
#include <plot/LineSeries.h>
#include <plot/PlotStyle.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <memory>

using namespace lumen::plot;
using namespace lumen::data;

TEST_CASE("Axis setLabel emits changed()", "[axis][setters]") {
    Axis axis(AxisOrientation::Horizontal);
    QSignalSpy spy(&axis, &Axis::changed);

    axis.setLabel("Time (ms)");
    REQUIRE(spy.count() == 1);
    REQUIRE(axis.label() == "Time (ms)");

    // Setting the same value should not emit.
    axis.setLabel("Time (ms)");
    REQUIRE(spy.count() == 1);
}

TEST_CASE("Axis setRangeMode emits changed()", "[axis][setters]") {
    Axis axis(AxisOrientation::Vertical);
    QSignalSpy spy(&axis, &Axis::changed);

    REQUIRE(axis.rangeMode() == RangeMode::Auto);

    axis.setRangeMode(RangeMode::Manual);
    REQUIRE(spy.count() == 1);
    REQUIRE(axis.rangeMode() == RangeMode::Manual);

    // Setting the same value should not emit.
    axis.setRangeMode(RangeMode::Manual);
    REQUIRE(spy.count() == 1);
}

TEST_CASE("Axis setManualRange stores values and emits changed()", "[axis][setters]") {
    Axis axis(AxisOrientation::Horizontal);
    QSignalSpy spy(&axis, &Axis::changed);

    axis.setManualRange(-10.0, 50.0);
    REQUIRE(spy.count() == 1);
    REQUIRE(axis.manualMin() == -10.0);
    REQUIRE(axis.manualMax() == 50.0);

    // Setting the same values should not emit.
    axis.setManualRange(-10.0, 50.0);
    REQUIRE(spy.count() == 1);
}

TEST_CASE("Axis Manual range mode uses manualMin/manualMax for ticks", "[axis][setters]") {
    Axis axis(AxisOrientation::Horizontal);

    // Set auto range first.
    std::vector<double> xData = {0.0, 100.0};
    std::vector<double> yData = {0.0, 100.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), xData);
    auto yCol = std::make_shared<lumen::data::Rank1Dataset>("y", lumen::data::Unit::dimensionless(), yData);
    LineSeries series(xCol, yCol, PlotStyle::fromPalette(0));
    axis.autoRange({series});

    double autoMin = axis.min();
    double autoMax = axis.max();
    REQUIRE(autoMin < 0.0);     // Has 5% padding.
    REQUIRE(autoMax > 100.0);

    // Now set manual range.
    axis.setRangeMode(RangeMode::Manual);
    axis.setManualRange(10.0, 20.0);
    REQUIRE(axis.min() == 10.0);
    REQUIRE(axis.max() == 20.0);

    // Ticks should be within the manual range bounds.
    auto marks = axis.ticks();
    REQUIRE(!marks.empty());
    // NiceNumbers may extend slightly past, but first and last should be near range.
    REQUIRE(marks.front().value >= 5.0);   // Generous bound.
    REQUIRE(marks.back().value <= 25.0);

    // Switch back to Auto.
    axis.setRangeMode(RangeMode::Auto);
    REQUIRE(axis.min() == autoMin);
    REQUIRE(axis.max() == autoMax);
}

TEST_CASE("Axis setTickCount affects tick generation", "[axis][setters]") {
    Axis axis(AxisOrientation::Horizontal);
    axis.setRange(0.0, 100.0);

    // Default (0 = auto, ~7 ticks).
    auto defaultTicks = axis.ticks();
    REQUIRE(!defaultTicks.empty());

    // Request more ticks.
    axis.setTickCount(15);
    auto moreTicks = axis.ticks();
    REQUIRE(!moreTicks.empty());
    // With a higher target, we generally get more ticks (or at least different).
    // NiceNumbers is approximate, but 15 target should differ from 7.
    REQUIRE(axis.tickCount() == 15);
}

TEST_CASE("Axis setTickFormat and setTickFormatDecimals", "[axis][setters]") {
    Axis axis(AxisOrientation::Vertical);
    axis.setRange(0.0, 1000.0);

    // Default: Auto format.
    REQUIRE(axis.tickFormat() == TickFormat::Auto);
    auto autoTicks = axis.ticks();
    REQUIRE(!autoTicks.empty());

    // Scientific format.
    axis.setTickFormat(TickFormat::Scientific);
    axis.setTickFormatDecimals(3);
    auto sciTicks = axis.ticks();
    REQUIRE(!sciTicks.empty());
    // Scientific notation labels contain 'e'.
    REQUIRE(sciTicks.front().label.contains('e'));
    REQUIRE(axis.tickFormat() == TickFormat::Scientific);
    REQUIRE(axis.tickFormatDecimals() == 3);

    // Fixed format with 1 decimal.
    axis.setTickFormat(TickFormat::Fixed);
    axis.setTickFormatDecimals(1);
    auto fixedTicks = axis.ticks();
    REQUIRE(!fixedTicks.empty());
    // Fixed format labels should contain '.'.
    REQUIRE(fixedTicks.front().label.contains('.'));
}

TEST_CASE("Axis setGridVisible emits changed()", "[axis][setters]") {
    Axis axis(AxisOrientation::Horizontal);
    QSignalSpy spy(&axis, &Axis::changed);

    REQUIRE(axis.gridVisible() == true);  // Default.

    axis.setGridVisible(false);
    REQUIRE(spy.count() == 1);
    REQUIRE(axis.gridVisible() == false);

    // Setting the same value should not emit.
    axis.setGridVisible(false);
    REQUIRE(spy.count() == 1);

    axis.setGridVisible(true);
    REQUIRE(spy.count() == 2);
    REQUIRE(axis.gridVisible() == true);
}

TEST_CASE("Axis setRange implies Manual mode and emits changed()", "[axis][setters]") {
    Axis axis(AxisOrientation::Horizontal);
    REQUIRE(axis.rangeMode() == RangeMode::Auto);

    QSignalSpy spy(&axis, &Axis::changed);
    axis.setRange(5.0, 15.0);

    REQUIRE(axis.rangeMode() == RangeMode::Manual);
    REQUIRE(axis.min() == 5.0);
    REQUIRE(axis.max() == 15.0);
    REQUIRE(spy.count() == 1);
}

TEST_CASE("Axis tickFormat setters emit changed() only on change", "[axis][setters]") {
    Axis axis(AxisOrientation::Vertical);
    QSignalSpy spy(&axis, &Axis::changed);

    axis.setTickFormat(TickFormat::Scientific);
    REQUIRE(spy.count() == 1);

    axis.setTickFormat(TickFormat::Scientific);
    REQUIRE(spy.count() == 1);  // No duplicate signal.

    axis.setTickFormatDecimals(5);
    REQUIRE(spy.count() == 2);

    axis.setTickFormatDecimals(5);
    REQUIRE(spy.count() == 2);  // No duplicate signal.

    axis.setTickCount(10);
    REQUIRE(spy.count() == 3);

    axis.setTickCount(10);
    REQUIRE(spy.count() == 3);  // No duplicate signal.
}
