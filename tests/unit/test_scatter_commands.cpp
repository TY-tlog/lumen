#include <catch2/catch_test_macros.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeScatterPropertiesCommand.h"
#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/PlotScene.h"
#include "plot/ScatterSeries.h"

#include <memory>
#include <vector>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeScatterPropertiesCommand;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::MarkerShape;
using lumen::plot::PlotScene;
using lumen::plot::ScatterSeries;

namespace {

struct ScatterFixture {
    std::shared_ptr<Rank1Dataset> xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    std::shared_ptr<Rank1Dataset> yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{4.0, 5.0, 6.0});
    std::unique_ptr<PlotScene> scene = std::make_unique<PlotScene>();

    ScatterFixture() {
        scene->addItem(std::make_unique<ScatterSeries>(xDs, yDs, Qt::red, "scatter1"));
    }
};

}  // namespace

TEST_CASE("ChangeScatterPropertiesCommand: execute applies new properties",
          "[core][scatter_command]") {
    ScatterFixture fix;

    auto cmd = std::make_unique<ChangeScatterPropertiesCommand>(
        fix.scene.get(), 0, QColor(Qt::blue), MarkerShape::Diamond, 12, false,
        "renamed", false);

    cmd->execute();

    auto* scatter = dynamic_cast<ScatterSeries*>(fix.scene->itemAt(0));
    REQUIRE(scatter != nullptr);
    CHECK(scatter->color() == QColor(Qt::blue));
    CHECK(scatter->markerShape() == MarkerShape::Diamond);
    CHECK(scatter->markerSize() == 12);
    CHECK_FALSE(scatter->filled());
    CHECK(scatter->name() == "renamed");
    CHECK_FALSE(scatter->isVisible());
}

TEST_CASE("ChangeScatterPropertiesCommand: undo restores old properties",
          "[core][scatter_command]") {
    ScatterFixture fix;

    auto* scatter = dynamic_cast<ScatterSeries*>(fix.scene->itemAt(0));
    REQUIRE(scatter != nullptr);

    auto cmd = std::make_unique<ChangeScatterPropertiesCommand>(
        fix.scene.get(), 0, QColor(Qt::green), MarkerShape::Square, 15, false,
        "changed", false);

    cmd->execute();
    CHECK(scatter->color() == QColor(Qt::green));
    CHECK(scatter->markerShape() == MarkerShape::Square);

    cmd->undo();
    CHECK(scatter->color() == QColor(Qt::red));
    CHECK(scatter->markerShape() == MarkerShape::Circle);
    CHECK(scatter->markerSize() == 6);
    CHECK(scatter->filled());
    CHECK(scatter->name() == "scatter1");
    CHECK(scatter->isVisible());
}

TEST_CASE("ChangeScatterPropertiesCommand: round-trip through CommandBus",
          "[core][scatter_command]") {
    ScatterFixture fix;

    CommandBus bus;

    bus.execute(std::make_unique<ChangeScatterPropertiesCommand>(
        fix.scene.get(), 0, QColor(Qt::cyan), MarkerShape::Triangle, 10, false,
        "bus_test", false));

    auto* scatter = dynamic_cast<ScatterSeries*>(fix.scene->itemAt(0));
    REQUIRE(scatter != nullptr);
    CHECK(scatter->color() == QColor(Qt::cyan));

    bus.undo();
    CHECK(scatter->color() == QColor(Qt::red));

    bus.redo();
    CHECK(scatter->color() == QColor(Qt::cyan));
}

TEST_CASE("ChangeScatterPropertiesCommand: description is non-empty",
          "[core][scatter_command]") {
    ScatterFixture fix;

    auto cmd = std::make_unique<ChangeScatterPropertiesCommand>(
        fix.scene.get(), 0, QColor(Qt::red), MarkerShape::Circle, 6, true,
        "scatter1", true);

    CHECK_FALSE(cmd->description().isEmpty());
}
