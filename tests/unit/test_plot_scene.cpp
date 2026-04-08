#include <catch2/catch_test_macros.hpp>

#include <data/Column.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

using namespace lumen::plot;
using namespace lumen::data;

TEST_CASE("PlotScene default is empty", "[plotscene]") {
    PlotScene scene;
    REQUIRE(scene.seriesCount() == 0);
    REQUIRE(scene.title().isEmpty());
}

TEST_CASE("PlotScene add and clear series", "[plotscene]") {
    std::vector<double> x = {1.0, 2.0, 3.0};
    std::vector<double> y = {4.0, 5.0, 6.0};
    Column xCol("x", x);
    Column yCol("y", y);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0), "s1"));
    REQUIRE(scene.seriesCount() == 1);

    scene.addSeries(LineSeries(&xCol, &yCol, PlotStyle::fromPalette(1), "s2"));
    REQUIRE(scene.seriesCount() == 2);

    scene.clearSeries();
    REQUIRE(scene.seriesCount() == 0);
}

TEST_CASE("PlotScene autoRange sets axes", "[plotscene]") {
    std::vector<double> x = {0.0, 10.0};
    std::vector<double> y = {-5.0, 5.0};
    Column xCol("x", x);
    Column yCol("y", y);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0)));
    scene.autoRange();

    REQUIRE(scene.xAxis().min() < 0.0);
    REQUIRE(scene.xAxis().max() > 10.0);
    REQUIRE(scene.yAxis().min() < -5.0);
    REQUIRE(scene.yAxis().max() > 5.0);
}

TEST_CASE("PlotScene computePlotArea margins", "[plotscene]") {
    PlotScene scene;
    QSizeF size(800.0, 600.0);

    auto area = scene.computePlotArea(size);

    // Left margin should be ~60.
    REQUIRE(area.x() > 40.0);
    // Bottom margin: area.bottom() < 600 - 30.
    REQUIRE(area.bottom() < 570.0);
    // Right margin: area.right() < 800.
    REQUIRE(area.right() < 800.0);
    // Positive size.
    REQUIRE(area.width() > 0.0);
    REQUIRE(area.height() > 0.0);
}

TEST_CASE("PlotScene title affects top margin", "[plotscene]") {
    PlotScene scene;
    QSizeF size(800.0, 600.0);

    auto areaNoTitle = scene.computePlotArea(size);

    scene.setTitle("My Plot");
    auto areaWithTitle = scene.computePlotArea(size);

    // With title, top margin is larger → plot area starts lower → smaller height.
    REQUIRE(areaWithTitle.y() > areaNoTitle.y());
    REQUIRE(areaWithTitle.height() < areaNoTitle.height());
}

TEST_CASE("PlotScene autoRange syncs ViewTransform", "[plotscene]") {
    std::vector<double> x = {0.0, 100.0};
    std::vector<double> y = {-50.0, 50.0};
    Column xCol("x", x);
    Column yCol("y", y);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0)));
    scene.autoRange();

    // ViewTransform should have the same range as the axes.
    REQUIRE(scene.viewTransform().xMin() == scene.xAxis().min());
    REQUIRE(scene.viewTransform().xMax() == scene.xAxis().max());
    REQUIRE(scene.viewTransform().yMin() == scene.yAxis().min());
    REQUIRE(scene.viewTransform().yMax() == scene.yAxis().max());
}
