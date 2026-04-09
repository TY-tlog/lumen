#include <catch2/catch_test_macros.hpp>

#include <QSignalSpy>

#include <plot/Legend.h>
#include <plot/PlotScene.h>

using namespace lumen::plot;

TEST_CASE("Legend default state", "[legend]") {
    Legend legend;
    REQUIRE(legend.position() == LegendPosition::TopRight);
    REQUIRE(legend.isVisible() == true);
}

TEST_CASE("Legend setPosition emits changed()", "[legend]") {
    Legend legend;
    QSignalSpy spy(&legend, &Legend::changed);

    legend.setPosition(LegendPosition::BottomLeft);
    REQUIRE(spy.count() == 1);
    REQUIRE(legend.position() == LegendPosition::BottomLeft);

    // Setting the same value should not emit.
    legend.setPosition(LegendPosition::BottomLeft);
    REQUIRE(spy.count() == 1);

    // All positions are settable.
    legend.setPosition(LegendPosition::TopLeft);
    REQUIRE(spy.count() == 2);
    REQUIRE(legend.position() == LegendPosition::TopLeft);

    legend.setPosition(LegendPosition::OutsideRight);
    REQUIRE(spy.count() == 3);
    REQUIRE(legend.position() == LegendPosition::OutsideRight);
}

TEST_CASE("Legend setVisible emits changed()", "[legend]") {
    Legend legend;
    QSignalSpy spy(&legend, &Legend::changed);

    legend.setVisible(false);
    REQUIRE(spy.count() == 1);
    REQUIRE(legend.isVisible() == false);

    // Setting the same value should not emit.
    legend.setVisible(false);
    REQUIRE(spy.count() == 1);

    legend.setVisible(true);
    REQUIRE(spy.count() == 2);
    REQUIRE(legend.isVisible() == true);
}

TEST_CASE("PlotScene owns a Legend instance", "[legend][plotscene]") {
    PlotScene scene;

    // Default state via PlotScene.
    REQUIRE(scene.legend().position() == LegendPosition::TopRight);
    REQUIRE(scene.legend().isVisible() == true);

    // Mutate via reference.
    scene.legend().setPosition(LegendPosition::BottomRight);
    REQUIRE(scene.legend().position() == LegendPosition::BottomRight);

    scene.legend().setVisible(false);
    REQUIRE(scene.legend().isVisible() == false);
}

TEST_CASE("Legend all five positions are distinct", "[legend]") {
    Legend legend;

    legend.setPosition(LegendPosition::TopLeft);
    REQUIRE(legend.position() == LegendPosition::TopLeft);

    legend.setPosition(LegendPosition::TopRight);
    REQUIRE(legend.position() == LegendPosition::TopRight);

    legend.setPosition(LegendPosition::BottomLeft);
    REQUIRE(legend.position() == LegendPosition::BottomLeft);

    legend.setPosition(LegendPosition::BottomRight);
    REQUIRE(legend.position() == LegendPosition::BottomRight);

    legend.setPosition(LegendPosition::OutsideRight);
    REQUIRE(legend.position() == LegendPosition::OutsideRight);
}
