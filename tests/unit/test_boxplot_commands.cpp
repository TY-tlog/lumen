#include <catch2/catch_test_macros.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeBoxPlotPropertiesCommand.h"
#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/BoxPlotSeries.h"
#include "plot/PlotScene.h"

#include <memory>
#include <vector>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeBoxPlotPropertiesCommand;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::BoxPlotSeries;
using lumen::plot::PlotScene;

namespace {

struct BoxFixture {
    std::shared_ptr<Rank1Dataset> ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(),
        std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0});
    std::unique_ptr<PlotScene> scene = std::make_unique<PlotScene>();

    BoxFixture() {
        scene->addItem(std::make_unique<BoxPlotSeries>(ds));
    }
};

}  // namespace

TEST_CASE("ChangeBoxPlotPropertiesCommand: execute applies new properties",
          "[core][boxplot_command]") {
    BoxFixture fix;
    auto cmd = std::make_unique<ChangeBoxPlotPropertiesCommand>(
        fix.scene.get(), 0,
        BoxPlotSeries::WhiskerRule::MinMax, true, false,
        QColor(Qt::green), "renamed", false);
    cmd->execute();
    auto* bp = dynamic_cast<BoxPlotSeries*>(fix.scene->itemAt(0));
    REQUIRE(bp != nullptr);
    CHECK(bp->whiskerRule() == BoxPlotSeries::WhiskerRule::MinMax);
    CHECK(bp->notched());
    CHECK_FALSE(bp->outliersVisible());
    CHECK(bp->fillColor() == QColor(Qt::green));
    CHECK(bp->name() == "renamed");
    CHECK_FALSE(bp->isVisible());
}

TEST_CASE("ChangeBoxPlotPropertiesCommand: undo restores old properties",
          "[core][boxplot_command]") {
    BoxFixture fix;
    auto* bp = dynamic_cast<BoxPlotSeries*>(fix.scene->itemAt(0));
    REQUIRE(bp != nullptr);
    QColor origColor = bp->fillColor();
    auto cmd = std::make_unique<ChangeBoxPlotPropertiesCommand>(
        fix.scene.get(), 0,
        BoxPlotSeries::WhiskerRule::Percentile, true, false,
        QColor(Qt::red), "changed", false);
    cmd->execute();
    CHECK(bp->fillColor() == QColor(Qt::red));
    cmd->undo();
    CHECK(bp->fillColor() == origColor);
    CHECK(bp->whiskerRule() == BoxPlotSeries::WhiskerRule::Tukey);
    CHECK_FALSE(bp->notched());
    CHECK(bp->outliersVisible());
    CHECK(bp->isVisible());
}

TEST_CASE("ChangeBoxPlotPropertiesCommand: round-trip through CommandBus",
          "[core][boxplot_command]") {
    BoxFixture fix;
    CommandBus bus;
    QColor origColor = dynamic_cast<BoxPlotSeries*>(fix.scene->itemAt(0))->fillColor();
    bus.execute(std::make_unique<ChangeBoxPlotPropertiesCommand>(
        fix.scene.get(), 0,
        BoxPlotSeries::WhiskerRule::MinMax, false, true,
        QColor(Qt::cyan), "bus_test", false));
    auto* bp = dynamic_cast<BoxPlotSeries*>(fix.scene->itemAt(0));
    REQUIRE(bp != nullptr);
    CHECK(bp->fillColor() == QColor(Qt::cyan));
    bus.undo();
    CHECK(bp->fillColor() == origColor);
    bus.redo();
    CHECK(bp->fillColor() == QColor(Qt::cyan));
}

TEST_CASE("ChangeBoxPlotPropertiesCommand: description is non-empty",
          "[core][boxplot_command]") {
    BoxFixture fix;
    auto cmd = std::make_unique<ChangeBoxPlotPropertiesCommand>(
        fix.scene.get(), 0,
        BoxPlotSeries::WhiskerRule::Tukey, false, true,
        QColor(70, 130, 180), "box1", true);
    CHECK_FALSE(cmd->description().isEmpty());
}
