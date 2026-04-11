#include <catch2/catch_test_macros.hpp>

#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

#include <memory>

using namespace lumen::plot;
using namespace lumen::data;

TEST_CASE("PlotScene default is empty", "[plotscene]") {
    PlotScene scene;
    REQUIRE(scene.seriesCount() == 0);
    REQUIRE(scene.title().isEmpty());
}

TEST_CASE("PlotScene add and clear series", "[plotscene]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{4.0, 5.0, 6.0});

    PlotScene scene;
    scene.addSeries(LineSeries(xDs, yDs, PlotStyle::fromPalette(0), "s1"));
    REQUIRE(scene.seriesCount() == 1);

    scene.addSeries(LineSeries(xDs, yDs, PlotStyle::fromPalette(1), "s2"));
    REQUIRE(scene.seriesCount() == 2);

    scene.clearSeries();
    REQUIRE(scene.seriesCount() == 0);
}

TEST_CASE("PlotScene autoRange sets axes", "[plotscene]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{0.0, 10.0});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{-5.0, 5.0});

    PlotScene scene;
    scene.addSeries(LineSeries(xDs, yDs, PlotStyle::fromPalette(0)));
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

    REQUIRE(area.x() > 0.0);
    REQUIRE(area.bottom() < size.height());
    REQUIRE(area.right() < 800.0);
    REQUIRE(area.width() > 0.0);
    REQUIRE(area.height() > 0.0);
}

TEST_CASE("PlotScene title affects top margin", "[plotscene]") {
    PlotScene scene;
    QSizeF size(800.0, 600.0);

    auto areaNoTitle = scene.computePlotArea(size);

    scene.setTitle("My Plot");
    auto areaWithTitle = scene.computePlotArea(size);

    REQUIRE(areaWithTitle.y() > areaNoTitle.y());
    REQUIRE(areaWithTitle.height() < areaNoTitle.height());
}

TEST_CASE("PlotScene autoRange syncs ViewTransform", "[plotscene]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{0.0, 100.0});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{-50.0, 50.0});

    PlotScene scene;
    scene.addSeries(LineSeries(xDs, yDs, PlotStyle::fromPalette(0)));
    scene.autoRange();

    REQUIRE(scene.viewTransform().xMin() == scene.xAxis().min());
    REQUIRE(scene.viewTransform().xMax() == scene.xAxis().max());
    REQUIRE(scene.viewTransform().yMin() == scene.yAxis().min());
    REQUIRE(scene.viewTransform().yMax() == scene.yAxis().max());
}
