#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Column.h>
#include <plot/CoordinateMapper.h>
#include <plot/HitTester.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

using namespace lumen::plot;
using namespace lumen::data;

namespace {

/// Helper: create a CoordinateMapper matching a scene after autoRange.
CoordinateMapper makeMapper(const PlotScene& scene, QSizeF widgetSize = {800, 600}) {
    QRectF plotArea = scene.computePlotArea(widgetSize);
    const auto& vt = scene.viewTransform();
    return CoordinateMapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(), plotArea);
}

}  // namespace

TEST_CASE("HitTester: point on a diagonal line returns correct series", "[hittester]") {
    // Series 0: diagonal from (0,0) to (10,10).
    std::vector<double> x = {0.0, 10.0};
    std::vector<double> y = {0.0, 10.0};
    Column xCol("x", x);
    Column yCol("y", y);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0), "diag"));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Map the midpoint (5, 5) to pixel space — should be exactly on the line.
    QPointF pixelMid = mapper.dataToPixel(5.0, 5.0);

    auto result = HitTester::hitTest(scene, mapper, pixelMid, 5.0);
    REQUIRE(result.has_value());
    CHECK(result->seriesIndex == 0);
    CHECK(result->pixelDistance < 1.0);
}

TEST_CASE("HitTester: point between two series returns nearest", "[hittester]") {
    // Series 0: horizontal line at y=0.
    std::vector<double> x = {0.0, 10.0};
    std::vector<double> y0 = {0.0, 0.0};
    // Series 1: horizontal line at y=10.
    std::vector<double> y1 = {10.0, 10.0};

    Column xCol("x", x);
    Column yCol0("y0", y0);
    Column yCol1("y1", y1);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol0, PlotStyle::fromPalette(0), "low"));
    scene.addSeries(LineSeries(&xCol, &yCol1, PlotStyle::fromPalette(1), "high"));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Point at y=2 (closer to series 0 at y=0 than series 1 at y=10).
    QPointF pixelNearLow = mapper.dataToPixel(5.0, 2.0);
    auto result = HitTester::hitTest(scene, mapper, pixelNearLow, 500.0);
    REQUIRE(result.has_value());
    CHECK(result->seriesIndex == 0);

    // Point at y=8 (closer to series 1 at y=10).
    QPointF pixelNearHigh = mapper.dataToPixel(5.0, 8.0);
    result = HitTester::hitTest(scene, mapper, pixelNearHigh, 500.0);
    REQUIRE(result.has_value());
    CHECK(result->seriesIndex == 1);
}

TEST_CASE("HitTester: point far from all series returns nullopt", "[hittester]") {
    std::vector<double> x = {0.0, 10.0};
    std::vector<double> y = {0.0, 10.0};
    Column xCol("x", x);
    Column yCol("y", y);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0)));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Point at pixel (0, 0) — far outside the plot area content.
    QPointF farPoint(0.0, 0.0);
    auto result = HitTester::hitTest(scene, mapper, farPoint, 5.0);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("HitTester: empty scene returns nullopt", "[hittester]") {
    PlotScene scene;
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    auto result = HitTester::hitTest(scene, mapper, QPointF(100.0, 100.0), 5.0);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("HitTester: three series, correct disambiguation", "[hittester]") {
    // Three horizontal lines at y=0, y=50, y=100.
    std::vector<double> x = {0.0, 100.0};
    std::vector<double> y0 = {0.0, 0.0};
    std::vector<double> y1 = {50.0, 50.0};
    std::vector<double> y2 = {100.0, 100.0};

    Column xCol("x", x);
    Column yCol0("y0", y0);
    Column yCol1("y1", y1);
    Column yCol2("y2", y2);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol0, PlotStyle::fromPalette(0), "bottom"));
    scene.addSeries(LineSeries(&xCol, &yCol1, PlotStyle::fromPalette(1), "middle"));
    scene.addSeries(LineSeries(&xCol, &yCol2, PlotStyle::fromPalette(2), "top"));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Test near each series.
    QPointF nearBottom = mapper.dataToPixel(50.0, 1.0);
    auto r0 = HitTester::hitTest(scene, mapper, nearBottom, 500.0);
    REQUIRE(r0.has_value());
    CHECK(r0->seriesIndex == 0);

    QPointF nearMiddle = mapper.dataToPixel(50.0, 49.0);
    auto r1 = HitTester::hitTest(scene, mapper, nearMiddle, 500.0);
    REQUIRE(r1.has_value());
    CHECK(r1->seriesIndex == 1);

    QPointF nearTop = mapper.dataToPixel(50.0, 99.0);
    auto r2 = HitTester::hitTest(scene, mapper, nearTop, 500.0);
    REQUIRE(r2.has_value());
    CHECK(r2->seriesIndex == 2);
}

TEST_CASE("HitTester: tolerance boundary", "[hittester]") {
    // Horizontal line at y=5 from x=0 to x=10.
    std::vector<double> x = {0.0, 10.0};
    std::vector<double> y = {5.0, 5.0};
    Column xCol("x", x);
    Column yCol("y", y);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0)));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Point exactly on the line — should always hit with any positive tolerance.
    QPointF onLine = mapper.dataToPixel(5.0, 5.0);
    auto result = HitTester::hitTest(scene, mapper, onLine, 0.5);
    REQUIRE(result.has_value());
    CHECK(result->seriesIndex == 0);
    CHECK(result->pixelDistance < 0.5);
}
