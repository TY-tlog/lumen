#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <memory>
#include <plot/CoordinateMapper.h>
#include <plot/HitTester.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

#include <cmath>
#include <limits>

using namespace lumen::plot;
using namespace lumen::data;

namespace {

constexpr QSizeF kWidgetSize{800.0, 600.0};

/// Helper: create a CoordinateMapper matching a scene after autoRange.
CoordinateMapper makeMapper(const PlotScene& scene,
                            QSizeF widgetSize = kWidgetSize) {
    QRectF plotArea = scene.computePlotArea(widgetSize);
    const auto& vt = scene.viewTransform();
    return CoordinateMapper(vt.xMin(), vt.xMax(), vt.yMin(), vt.yMax(),
                            plotArea);
}

}  // namespace

TEST_CASE("hitTestPoint: nearest sample on simple data", "[hittestpoint]") {
    // x = [0, 1, 2, 3, 4], y = [0, 10, 20, 30, 40]
    std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
    std::vector<double> y = {0.0, 10.0, 20.0, 30.0, 40.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), x);
    auto yCol = std::make_shared<lumen::data::Rank1Dataset>("y", lumen::data::Unit::dimensionless(), y);

    PlotScene scene;
    scene.addSeries(LineSeries(xCol, yCol, PlotStyle::fromPalette(0), "sig"));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Query at the pixel position of sample index 2 (data = (2, 20)).
    QPointF pixelAtSample2 = mapper.dataToPixel(2.0, 20.0);
    auto result = HitTester::hitTestPoint(scene, mapper, pixelAtSample2, 20.0);

    REQUIRE(result.has_value());
    CHECK(result->seriesIndex == 0);
    CHECK(result->sampleIndex == 2);
    CHECK(result->dataPoint.x() == 2.0);
    CHECK(result->dataPoint.y() == 20.0);
    CHECK(result->pixelDistance < 1.0);
}

TEST_CASE("hitTestPoint: between two samples returns closer one",
          "[hittestpoint]") {
    // x = [0, 10, 20, 30], y = [0, 0, 0, 0] -- horizontal line, evenly spaced.
    std::vector<double> x = {0.0, 10.0, 20.0, 30.0};
    std::vector<double> y = {0.0, 0.0, 0.0, 0.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), x);
    auto yCol = std::make_shared<lumen::data::Rank1Dataset>("y", lumen::data::Unit::dimensionless(), y);

    PlotScene scene;
    scene.addSeries(LineSeries(xCol, yCol, PlotStyle::fromPalette(0), "flat"));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Query slightly closer to sample 1 (x=10) than sample 2 (x=20).
    // Midpoint would be x=15; query at x=12, which is closer to sample 1.
    QPointF pixelNearSample1 = mapper.dataToPixel(12.0, 0.0);
    auto result =
        HitTester::hitTestPoint(scene, mapper, pixelNearSample1, 500.0);

    REQUIRE(result.has_value());
    CHECK(result->seriesIndex == 0);
    CHECK(result->sampleIndex == 1);
    CHECK(result->dataPoint.x() == 10.0);

    // Query at x=18, closer to sample 2 (x=20).
    QPointF pixelNearSample2 = mapper.dataToPixel(18.0, 0.0);
    auto result2 =
        HitTester::hitTestPoint(scene, mapper, pixelNearSample2, 500.0);

    REQUIRE(result2.has_value());
    CHECK(result2->seriesIndex == 0);
    CHECK(result2->sampleIndex == 2);
    CHECK(result2->dataPoint.x() == 20.0);
}

TEST_CASE("hitTestPoint: NaN sample is skipped", "[hittestpoint]") {
    // x = [0, 1, 2, 3], y = [0, NaN, 20, 30]
    std::vector<double> x = {0.0, 1.0, 2.0, 3.0};
    std::vector<double> y = {0.0, std::numeric_limits<double>::quiet_NaN(),
                             20.0, 30.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), x);
    auto yCol = std::make_shared<lumen::data::Rank1Dataset>("y", lumen::data::Unit::dimensionless(), y);

    PlotScene scene;
    scene.addSeries(
        LineSeries(xCol, yCol, PlotStyle::fromPalette(0), "nan_test"));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Query at the pixel position of sample 1 (x=1, y=NaN).
    // Since sample 1 is NaN, the result should be sample 0 or 2 instead.
    QPointF pixelAtSample1 = mapper.dataToPixel(1.0, 10.0);
    auto result =
        HitTester::hitTestPoint(scene, mapper, pixelAtSample1, 500.0);

    REQUIRE(result.has_value());
    // Must NOT be sample index 1 (which is NaN).
    CHECK(result->sampleIndex != 1);
    CHECK_FALSE(std::isnan(result->dataPoint.x()));
    CHECK_FALSE(std::isnan(result->dataPoint.y()));
}

TEST_CASE("hitTestPoint: outside max distance returns nullopt",
          "[hittestpoint]") {
    std::vector<double> x = {0.0, 1.0};
    std::vector<double> y = {0.0, 0.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), x);
    auto yCol = std::make_shared<lumen::data::Rank1Dataset>("y", lumen::data::Unit::dimensionless(), y);

    PlotScene scene;
    scene.addSeries(LineSeries(xCol, yCol, PlotStyle::fromPalette(0)));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Query at a pixel position very far from the data.
    // Use a point in the upper-right extreme of the plot area, far from
    // the data which is near the bottom-left for y=0 (since Y is inverted).
    QRectF plotArea = scene.computePlotArea(kWidgetSize);
    QPointF farPoint(plotArea.left() + plotArea.width() / 2.0, plotArea.top());

    // Use a very small max distance so nothing matches.
    auto result = HitTester::hitTestPoint(scene, mapper, farPoint, 1.0);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("hitTestPoint: multiple series returns nearest series",
          "[hittestpoint]") {
    // Series 0: y = 0 (bottom).
    // Series 1: y = 100 (top).
    std::vector<double> x = {0.0, 5.0, 10.0};
    std::vector<double> y0 = {0.0, 0.0, 0.0};
    std::vector<double> y1 = {100.0, 100.0, 100.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), x);
    auto yCol0 = std::make_shared<lumen::data::Rank1Dataset>("y0", lumen::data::Unit::dimensionless(), y0);
    auto yCol1 = std::make_shared<lumen::data::Rank1Dataset>("y1", lumen::data::Unit::dimensionless(), y1);

    PlotScene scene;
    scene.addSeries(
        LineSeries(xCol, yCol0, PlotStyle::fromPalette(0), "bottom"));
    scene.addSeries(
        LineSeries(xCol, yCol1, PlotStyle::fromPalette(1), "top"));
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    // Query near series 1 (y=100), at x=5.
    QPointF pixelNearTop = mapper.dataToPixel(5.0, 98.0);
    auto result =
        HitTester::hitTestPoint(scene, mapper, pixelNearTop, 500.0);

    REQUIRE(result.has_value());
    CHECK(result->seriesIndex == 1);
    CHECK(result->dataPoint.y() == 100.0);

    // Query near series 0 (y=0), at x=5.
    QPointF pixelNearBottom = mapper.dataToPixel(5.0, 2.0);
    auto result2 =
        HitTester::hitTestPoint(scene, mapper, pixelNearBottom, 500.0);

    REQUIRE(result2.has_value());
    CHECK(result2->seriesIndex == 0);
    CHECK(result2->dataPoint.y() == 0.0);
}

TEST_CASE("hitTestPoint: empty scene returns nullopt", "[hittestpoint]") {
    PlotScene scene;
    scene.autoRange();

    CoordinateMapper mapper = makeMapper(scene);

    auto result =
        HitTester::hitTestPoint(scene, mapper, QPointF(100.0, 100.0), 20.0);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("hitTestPoint: invisible series is skipped", "[hittestpoint]") {
    // Two series sharing x, with y values close together so pixel distance
    // is comfortably within any reasonable max.
    std::vector<double> x = {0.0, 5.0, 10.0};
    std::vector<double> y0 = {50.0, 50.0, 50.0};
    std::vector<double> y1 = {48.0, 48.0, 48.0};
    auto xCol = std::make_shared<lumen::data::Rank1Dataset>("x", lumen::data::Unit::dimensionless(), x);
    auto yCol0 = std::make_shared<lumen::data::Rank1Dataset>("y0", lumen::data::Unit::dimensionless(), y0);
    auto yCol1 = std::make_shared<lumen::data::Rank1Dataset>("y1", lumen::data::Unit::dimensionless(), y1);

    PlotScene scene;
    scene.addSeries(
        LineSeries(xCol, yCol0, PlotStyle::fromPalette(0), "hidden"));
    scene.addSeries(
        LineSeries(xCol, yCol1, PlotStyle::fromPalette(1), "visible"));
    scene.autoRange();

    // Hide series 0.
    scene.seriesAt(0).setVisible(false);

    CoordinateMapper mapper = makeMapper(scene);

    // Query exactly at the hidden series data point (y=50). Since series 0
    // is invisible, the only candidate is series 1 (y=48).
    QPointF pixelNearHidden = mapper.dataToPixel(5.0, 50.0);
    auto result =
        HitTester::hitTestPoint(scene, mapper, pixelNearHidden, 5000.0);

    REQUIRE(result.has_value());
    // Must return series 1 since series 0 is invisible.
    CHECK(result->seriesIndex == 1);
}
