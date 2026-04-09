#include <catch2/catch_test_macros.hpp>

#include <plot/PlotScene.h>

#include <QFont>

using namespace lumen::plot;

TEST_CASE("PlotScene title font defaults", "[plotscene][title]") {
    PlotScene scene;

    // Default values from tokens::typography::title3.
    REQUIRE(scene.titleFontPx() == 17);
    REQUIRE(scene.titleWeight() == QFont::DemiBold);
}

TEST_CASE("PlotScene setTitleFontPx stores value", "[plotscene][title]") {
    PlotScene scene;

    scene.setTitleFontPx(24);
    REQUIRE(scene.titleFontPx() == 24);

    scene.setTitleFontPx(11);
    REQUIRE(scene.titleFontPx() == 11);
}

TEST_CASE("PlotScene setTitleWeight stores value", "[plotscene][title]") {
    PlotScene scene;

    scene.setTitleWeight(QFont::Bold);
    REQUIRE(scene.titleWeight() == QFont::Bold);

    scene.setTitleWeight(QFont::Normal);
    REQUIRE(scene.titleWeight() == QFont::Normal);

    scene.setTitleWeight(QFont::DemiBold);
    REQUIRE(scene.titleWeight() == QFont::DemiBold);
}

TEST_CASE("PlotScene title text and font are independent", "[plotscene][title]") {
    PlotScene scene;

    scene.setTitle("Voltage vs Time");
    scene.setTitleFontPx(22);
    scene.setTitleWeight(QFont::Bold);

    REQUIRE(scene.title() == "Voltage vs Time");
    REQUIRE(scene.titleFontPx() == 22);
    REQUIRE(scene.titleWeight() == QFont::Bold);

    // Changing title text does not affect font.
    scene.setTitle("Current vs Time");
    REQUIRE(scene.titleFontPx() == 22);
    REQUIRE(scene.titleWeight() == QFont::Bold);
}
