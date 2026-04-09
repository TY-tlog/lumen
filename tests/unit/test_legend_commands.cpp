#include <catch2/catch_test_macros.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeLegendCommand.h"
#include "plot/Legend.h"

#include <memory>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeLegendCommand;
using lumen::plot::Legend;
using lumen::plot::LegendPosition;

TEST_CASE("ChangeLegendCommand: execute changes position",
          "[core][legend_command]") {
    Legend legend;
    CHECK(legend.position() == LegendPosition::TopRight);

    auto cmd = std::make_unique<ChangeLegendCommand>(
        &legend, LegendPosition::BottomLeft, true);

    cmd->execute();

    CHECK(legend.position() == LegendPosition::BottomLeft);
}

TEST_CASE("ChangeLegendCommand: execute changes visibility",
          "[core][legend_command]") {
    Legend legend;
    CHECK(legend.isVisible());

    auto cmd = std::make_unique<ChangeLegendCommand>(
        &legend, LegendPosition::TopRight, false);

    cmd->execute();

    CHECK_FALSE(legend.isVisible());
}

TEST_CASE("ChangeLegendCommand: undo restores position and visibility",
          "[core][legend_command]") {
    Legend legend;
    CHECK(legend.position() == LegendPosition::TopRight);
    CHECK(legend.isVisible());

    auto cmd = std::make_unique<ChangeLegendCommand>(
        &legend, LegendPosition::OutsideRight, false);

    cmd->execute();
    CHECK(legend.position() == LegendPosition::OutsideRight);
    CHECK_FALSE(legend.isVisible());

    cmd->undo();
    CHECK(legend.position() == LegendPosition::TopRight);
    CHECK(legend.isVisible());
}

TEST_CASE("ChangeLegendCommand: round-trip through CommandBus",
          "[core][legend_command]") {
    Legend legend;

    CommandBus bus;

    bus.execute(std::make_unique<ChangeLegendCommand>(
        &legend, LegendPosition::BottomRight, false));

    CHECK(legend.position() == LegendPosition::BottomRight);
    CHECK_FALSE(legend.isVisible());

    bus.undo();
    CHECK(legend.position() == LegendPosition::TopRight);
    CHECK(legend.isVisible());

    bus.redo();
    CHECK(legend.position() == LegendPosition::BottomRight);
    CHECK_FALSE(legend.isVisible());
}

TEST_CASE("ChangeLegendCommand: description is non-empty",
          "[core][legend_command]") {
    Legend legend;

    auto cmd = std::make_unique<ChangeLegendCommand>(
        &legend, LegendPosition::TopLeft, true);

    CHECK_FALSE(cmd->description().isEmpty());
}
