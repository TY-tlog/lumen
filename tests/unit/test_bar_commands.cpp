#include <catch2/catch_test_macros.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeBarPropertiesCommand.h"
#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/BarSeries.h"
#include "plot/PlotScene.h"

#include <memory>
#include <vector>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeBarPropertiesCommand;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::BarSeries;
using lumen::plot::PlotScene;

namespace {

struct BarFixture {
    std::shared_ptr<Rank1Dataset> xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    std::shared_ptr<Rank1Dataset> yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{4.0, 5.0, 6.0});
    std::unique_ptr<PlotScene> scene = std::make_unique<PlotScene>();

    BarFixture() {
        scene->addItem(std::make_unique<BarSeries>(xDs, yDs, Qt::red, "bar1"));
    }
};

}  // namespace

TEST_CASE("ChangeBarPropertiesCommand: execute applies new properties", "[core][bar_command]") {
    BarFixture fix;
    auto cmd = std::make_unique<ChangeBarPropertiesCommand>(
        fix.scene.get(), 0, QColor(Qt::blue), QColor(Qt::black), 0.5, "renamed", false);
    cmd->execute();
    auto* bar = dynamic_cast<BarSeries*>(fix.scene->itemAt(0));
    REQUIRE(bar != nullptr);
    CHECK(bar->fillColor() == QColor(Qt::blue));
    CHECK(bar->outlineColor() == QColor(Qt::black));
    CHECK(bar->barWidth() == 0.5);
    CHECK(bar->name() == "renamed");
    CHECK_FALSE(bar->isVisible());
}

TEST_CASE("ChangeBarPropertiesCommand: undo restores old properties", "[core][bar_command]") {
    BarFixture fix;
    auto* bar = dynamic_cast<BarSeries*>(fix.scene->itemAt(0));
    REQUIRE(bar != nullptr);
    auto cmd = std::make_unique<ChangeBarPropertiesCommand>(
        fix.scene.get(), 0, QColor(Qt::green), QColor(Qt::darkGray), 0.3, "changed", false);
    cmd->execute();
    CHECK(bar->fillColor() == QColor(Qt::green));
    cmd->undo();
    CHECK(bar->fillColor() == QColor(Qt::red));
    CHECK(bar->outlineColor() == QColor(Qt::transparent));
    CHECK(bar->barWidth() == 0.8);
    CHECK(bar->name() == "bar1");
    CHECK(bar->isVisible());
}

TEST_CASE("ChangeBarPropertiesCommand: round-trip through CommandBus", "[core][bar_command]") {
    BarFixture fix;
    CommandBus bus;
    bus.execute(std::make_unique<ChangeBarPropertiesCommand>(
        fix.scene.get(), 0, QColor(Qt::cyan), QColor(Qt::yellow), 0.6, "bus_test", false));
    auto* bar = dynamic_cast<BarSeries*>(fix.scene->itemAt(0));
    REQUIRE(bar != nullptr);
    CHECK(bar->fillColor() == QColor(Qt::cyan));
    bus.undo();
    CHECK(bar->fillColor() == QColor(Qt::red));
    bus.redo();
    CHECK(bar->fillColor() == QColor(Qt::cyan));
}

TEST_CASE("ChangeBarPropertiesCommand: description is non-empty", "[core][bar_command]") {
    BarFixture fix;
    auto cmd = std::make_unique<ChangeBarPropertiesCommand>(
        fix.scene.get(), 0, QColor(Qt::red), QColor(Qt::transparent), 0.8, "bar1", true);
    CHECK_FALSE(cmd->description().isEmpty());
}
