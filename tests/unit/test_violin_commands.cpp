#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeViolinPropertiesCommand.h"
#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/PlotScene.h"
#include "plot/ViolinSeries.h"

#include <memory>
#include <vector>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeViolinPropertiesCommand;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::PlotScene;
using lumen::plot::ViolinSeries;
using Catch::Matchers::WithinAbs;

namespace {

struct ViolinFixture {
    std::shared_ptr<Rank1Dataset> ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(),
        std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
    std::unique_ptr<PlotScene> scene = std::make_unique<PlotScene>();

    ViolinFixture() {
        scene->addItem(std::make_unique<ViolinSeries>(ds));
    }
};

}  // namespace

TEST_CASE("ChangeViolinPropertiesCommand: execute applies new properties",
          "[core][violin_command]") {
    ViolinFixture fix;
    auto cmd = std::make_unique<ChangeViolinPropertiesCommand>(
        fix.scene.get(), 0, 2.5, false, true,
        QColor(Qt::green), "renamed", false);
    cmd->execute();
    auto* vio = dynamic_cast<ViolinSeries*>(fix.scene->itemAt(0));
    REQUIRE(vio != nullptr);
    CHECK_THAT(vio->kdeBandwidth(), WithinAbs(2.5, 1e-9));
    CHECK_FALSE(vio->kdeBandwidthAuto());
    CHECK(vio->split());
    CHECK(vio->fillColor() == QColor(Qt::green));
    CHECK(vio->name() == "renamed");
    CHECK_FALSE(vio->isVisible());
}

TEST_CASE("ChangeViolinPropertiesCommand: undo restores old properties",
          "[core][violin_command]") {
    ViolinFixture fix;
    auto* vio = dynamic_cast<ViolinSeries*>(fix.scene->itemAt(0));
    REQUIRE(vio != nullptr);
    QColor origColor = vio->fillColor();
    auto cmd = std::make_unique<ChangeViolinPropertiesCommand>(
        fix.scene.get(), 0, 3.0, false, true,
        QColor(Qt::red), "changed", false);
    cmd->execute();
    CHECK(vio->fillColor() == QColor(Qt::red));
    cmd->undo();
    CHECK(vio->fillColor() == origColor);
    CHECK(vio->kdeBandwidthAuto());
    CHECK_FALSE(vio->split());
    CHECK(vio->isVisible());
}

TEST_CASE("ChangeViolinPropertiesCommand: round-trip through CommandBus",
          "[core][violin_command]") {
    ViolinFixture fix;
    CommandBus bus;
    QColor origColor = dynamic_cast<ViolinSeries*>(fix.scene->itemAt(0))->fillColor();
    bus.execute(std::make_unique<ChangeViolinPropertiesCommand>(
        fix.scene.get(), 0, 1.5, false, true,
        QColor(Qt::cyan), "bus_test", false));
    auto* vio = dynamic_cast<ViolinSeries*>(fix.scene->itemAt(0));
    REQUIRE(vio != nullptr);
    CHECK(vio->fillColor() == QColor(Qt::cyan));
    bus.undo();
    CHECK(vio->fillColor() == origColor);
    bus.redo();
    CHECK(vio->fillColor() == QColor(Qt::cyan));
}

TEST_CASE("ChangeViolinPropertiesCommand: description is non-empty",
          "[core][violin_command]") {
    ViolinFixture fix;
    auto cmd = std::make_unique<ChangeViolinPropertiesCommand>(
        fix.scene.get(), 0, 1.0, true, false,
        QColor(70, 130, 180), "vio1", true);
    CHECK_FALSE(cmd->description().isEmpty());
}
