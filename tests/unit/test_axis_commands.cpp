#include <catch2/catch_test_macros.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeAxisPropertiesCommand.h"
#include "plot/Axis.h"

#include <memory>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeAxisPropertiesCommand;
using lumen::plot::Axis;
using lumen::plot::AxisOrientation;
using lumen::plot::RangeMode;
using lumen::plot::TickFormat;

TEST_CASE("ChangeAxisPropertiesCommand: execute changes axis label",
          "[core][axis_command]") {
    Axis axis(AxisOrientation::Horizontal);
    axis.setLabel("Original");

    auto cmd = std::make_unique<ChangeAxisPropertiesCommand>(
        &axis, "New Label", RangeMode::Auto, 0.0, 1.0, 0, TickFormat::Auto, 2,
        true);

    cmd->execute();

    CHECK(axis.label() == "New Label");
}

TEST_CASE("ChangeAxisPropertiesCommand: undo restores original label",
          "[core][axis_command]") {
    Axis axis(AxisOrientation::Horizontal);
    axis.setLabel("Original");

    auto cmd = std::make_unique<ChangeAxisPropertiesCommand>(
        &axis, "New Label", RangeMode::Auto, 0.0, 1.0, 0, TickFormat::Auto, 2,
        true);

    cmd->execute();
    CHECK(axis.label() == "New Label");

    cmd->undo();
    CHECK(axis.label() == "Original");
}

TEST_CASE(
    "ChangeAxisPropertiesCommand: execute changes range mode to Manual with "
    "min/max",
    "[core][axis_command]") {
    Axis axis(AxisOrientation::Vertical);
    CHECK(axis.rangeMode() == RangeMode::Auto);

    auto cmd = std::make_unique<ChangeAxisPropertiesCommand>(
        &axis, "", RangeMode::Manual, -10.0, 50.0, 0, TickFormat::Auto, 2,
        true);

    cmd->execute();

    CHECK(axis.rangeMode() == RangeMode::Manual);
    CHECK(axis.manualMin() == -10.0);
    CHECK(axis.manualMax() == 50.0);
}

TEST_CASE("ChangeAxisPropertiesCommand: undo restores Auto mode",
          "[core][axis_command]") {
    Axis axis(AxisOrientation::Vertical);
    CHECK(axis.rangeMode() == RangeMode::Auto);

    auto cmd = std::make_unique<ChangeAxisPropertiesCommand>(
        &axis, "Y", RangeMode::Manual, -10.0, 50.0, 5, TickFormat::Fixed, 3,
        false);

    cmd->execute();
    CHECK(axis.rangeMode() == RangeMode::Manual);
    CHECK(axis.tickCount() == 5);
    CHECK(axis.tickFormat() == TickFormat::Fixed);
    CHECK(axis.tickFormatDecimals() == 3);
    CHECK_FALSE(axis.gridVisible());

    cmd->undo();
    CHECK(axis.rangeMode() == RangeMode::Auto);
    CHECK(axis.tickCount() == 0);
    CHECK(axis.tickFormat() == TickFormat::Auto);
    CHECK(axis.tickFormatDecimals() == 2);
    CHECK(axis.gridVisible());
    CHECK(axis.label().isEmpty());
}

TEST_CASE("ChangeAxisPropertiesCommand: full round-trip through CommandBus",
          "[core][axis_command]") {
    Axis axis(AxisOrientation::Horizontal);
    axis.setLabel("Time (s)");
    axis.setGridVisible(true);

    CommandBus bus;

    bus.execute(std::make_unique<ChangeAxisPropertiesCommand>(
        &axis, "Frequency (Hz)", RangeMode::Manual, 0.0, 100.0, 10,
        TickFormat::Scientific, 4, false));

    CHECK(axis.label() == "Frequency (Hz)");
    CHECK(axis.rangeMode() == RangeMode::Manual);
    CHECK(axis.manualMin() == 0.0);
    CHECK(axis.manualMax() == 100.0);
    CHECK(axis.tickCount() == 10);
    CHECK(axis.tickFormat() == TickFormat::Scientific);
    CHECK(axis.tickFormatDecimals() == 4);
    CHECK_FALSE(axis.gridVisible());

    bus.undo();

    CHECK(axis.label() == "Time (s)");
    CHECK(axis.rangeMode() == RangeMode::Auto);
    CHECK(axis.tickCount() == 0);
    CHECK(axis.tickFormat() == TickFormat::Auto);
    CHECK(axis.tickFormatDecimals() == 2);
    CHECK(axis.gridVisible());

    bus.redo();

    CHECK(axis.label() == "Frequency (Hz)");
    CHECK(axis.rangeMode() == RangeMode::Manual);
    CHECK_FALSE(axis.gridVisible());
}

TEST_CASE("ChangeAxisPropertiesCommand: description is non-empty",
          "[core][axis_command]") {
    Axis axis(AxisOrientation::Horizontal);

    auto cmd = std::make_unique<ChangeAxisPropertiesCommand>(
        &axis, "", RangeMode::Auto, 0.0, 1.0, 0, TickFormat::Auto, 2, true);

    CHECK_FALSE(cmd->description().isEmpty());
}
