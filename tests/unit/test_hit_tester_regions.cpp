#include <catch2/catch_test_macros.hpp>

#include <data/Column.h>
#include <plot/CoordinateMapper.h>
#include <plot/HitTester.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

using namespace lumen::plot;
using namespace lumen::data;

namespace {

constexpr QSizeF kWidgetSize{800.0, 600.0};

/// Helper: create a scene with two series (so a legend is drawn) and a title.
PlotScene makeSceneWithTitleAndLegend() {
    std::vector<double> x = {0.0, 10.0};
    std::vector<double> y0 = {0.0, 10.0};
    std::vector<double> y1 = {5.0, 15.0};
    Column xCol("x", x);
    Column yCol0("y0", y0);
    Column yCol1("y1", y1);

    PlotScene scene;
    scene.setTitle("Test Plot");
    scene.addSeries(LineSeries(&xCol, &yCol0, PlotStyle::fromPalette(0), "Series A"));
    scene.addSeries(LineSeries(&xCol, &yCol1, PlotStyle::fromPalette(1), "Series B"));
    scene.autoRange();
    return scene;
}

/// Helper: create a scene with no title and one series (no legend).
PlotScene makeSceneNoTitleNoLegend() {
    std::vector<double> x = {0.0, 10.0};
    std::vector<double> y = {0.0, 10.0};
    Column xCol("x", x);
    Column yCol("y", y);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0), "Only"));
    scene.autoRange();
    return scene;
}

}  // namespace

TEST_CASE("hitNonSeriesElement: point in title area returns Title", "[hittester][regions]") {
    auto scene = makeSceneWithTitleAndLegend();
    QRectF plotArea = scene.computePlotArea(kWidgetSize);

    // Place point in the middle of the title area (above plotArea).
    QPointF titlePoint(plotArea.center().x(), plotArea.top() / 2.0);
    auto result = HitTester::hitNonSeriesElement(scene, kWidgetSize, titlePoint);
    CHECK(result.kind == HitKind::Title);
}

TEST_CASE("hitNonSeriesElement: point in X axis band returns XAxis", "[hittester][regions]") {
    auto scene = makeSceneWithTitleAndLegend();
    QRectF plotArea = scene.computePlotArea(kWidgetSize);

    // Place point below the plot area, centered horizontally.
    QPointF xAxisPoint(plotArea.center().x(), plotArea.bottom() + 10.0);
    auto result = HitTester::hitNonSeriesElement(scene, kWidgetSize, xAxisPoint);
    CHECK(result.kind == HitKind::XAxis);
}

TEST_CASE("hitNonSeriesElement: point in Y axis band returns YAxis", "[hittester][regions]") {
    auto scene = makeSceneWithTitleAndLegend();
    QRectF plotArea = scene.computePlotArea(kWidgetSize);

    // Place point to the left of the plot area, centered vertically.
    QPointF yAxisPoint(plotArea.left() / 2.0, plotArea.center().y());
    auto result = HitTester::hitNonSeriesElement(scene, kWidgetSize, yAxisPoint);
    CHECK(result.kind == HitKind::YAxis);
}

TEST_CASE("hitNonSeriesElement: point inside plot area returns PlotArea", "[hittester][regions]") {
    auto scene = makeSceneNoTitleNoLegend();
    QRectF plotArea = scene.computePlotArea(kWidgetSize);

    // Place point in the center of the plot area.
    QPointF centerPoint = plotArea.center();
    auto result = HitTester::hitNonSeriesElement(scene, kWidgetSize, centerPoint);
    CHECK(result.kind == HitKind::PlotArea);
}

TEST_CASE("hitNonSeriesElement: point in legend area returns Legend", "[hittester][regions]") {
    auto scene = makeSceneWithTitleAndLegend();
    QRectF plotArea = scene.computePlotArea(kWidgetSize);

    // The legend is at the top-right corner of the plot area.
    // Place point a few pixels inside from the top-right.
    QPointF legendPoint(plotArea.right() - 20.0, plotArea.top() + 20.0);
    auto result = HitTester::hitNonSeriesElement(scene, kWidgetSize, legendPoint);
    CHECK(result.kind == HitKind::Legend);
}

TEST_CASE("hitNonSeriesElement: precedence — title wins over Y axis overlap", "[hittester][regions]") {
    auto scene = makeSceneWithTitleAndLegend();
    QRectF plotArea = scene.computePlotArea(kWidgetSize);

    // At x=plotArea.left(), y = small value above plotArea.top().
    // This is at the boundary of title area and left margin.
    // Title region spans plotArea.left() to plotArea.right() in x,
    // so a point at (plotArea.left() + 1, plotArea.top() - 1) is in the title rect.
    QPointF overlapPoint(plotArea.left() + 1.0, plotArea.top() / 2.0);
    auto result = HitTester::hitNonSeriesElement(scene, kWidgetSize, overlapPoint);
    CHECK(result.kind == HitKind::Title);
}

TEST_CASE("hitNonSeriesElement: no title — title area returns None or axis", "[hittester][regions]") {
    auto scene = makeSceneNoTitleNoLegend();
    QRectF plotArea = scene.computePlotArea(kWidgetSize);

    // Point above the plot area when no title is set.
    // With no title, the title check is skipped. The point is outside all defined
    // regions (above plotArea but not in title/x-axis/y-axis/plotArea).
    QPointF abovePoint(plotArea.center().x(), 2.0);
    auto result = HitTester::hitNonSeriesElement(scene, kWidgetSize, abovePoint);
    // Without a title, a point above the plot area is not in any region.
    CHECK(result.kind == HitKind::None);
}

TEST_CASE("hitNonSeriesElement: point outside all regions returns None", "[hittester][regions]") {
    auto scene = makeSceneNoTitleNoLegend();

    // Far outside the widget bounds.
    QPointF outside(-50.0, -50.0);
    auto result = HitTester::hitNonSeriesElement(scene, kWidgetSize, outside);
    CHECK(result.kind == HitKind::None);
}
