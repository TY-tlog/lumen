#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeContourPropertiesCommand.h"
#include "data/CoordinateArray.h"
#include "data/Dimension.h"
#include "data/Grid2D.h"
#include "data/Unit.h"
#include "plot/ContourPlot.h"
#include "plot/PlotScene.h"

#include <memory>
#include <vector>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeContourPropertiesCommand;
using lumen::data::CoordinateArray;
using lumen::data::Dimension;
using lumen::data::Grid2D;
using lumen::data::Unit;
using lumen::plot::ContourPlot;
using lumen::plot::PlotScene;
using Catch::Matchers::WithinAbs;

namespace {

std::shared_ptr<Grid2D> makeTestGrid()
{
    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), 4,
                   CoordinateArray(0.0, 1.0, 4)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), 4,
                   CoordinateArray(0.0, 1.0, 4)};
    std::vector<double> data(16);
    for (std::size_t i = 0; i < 16; ++i)
        data[i] = static_cast<double>(i);
    return std::make_shared<Grid2D>(
        QStringLiteral("test"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

struct ContourFixture {
    std::shared_ptr<Grid2D> grid = makeTestGrid();
    std::unique_ptr<PlotScene> scene = std::make_unique<PlotScene>();

    ContourFixture() {
        scene->addItem(std::make_unique<ContourPlot>(grid));
    }
};

}  // namespace

TEST_CASE("ChangeContourPropertiesCommand: execute applies new properties",
          "[core][contour_command]") {
    ContourFixture fix;
    std::vector<double> levels = {2.0, 5.0, 8.0};
    auto cmd = std::make_unique<ChangeContourPropertiesCommand>(
        fix.scene.get(), 0,
        levels, 0,
        QColor(Qt::red), 2.0, true,
        "renamed", false);
    cmd->execute();
    auto* cp = dynamic_cast<ContourPlot*>(fix.scene->itemAt(0));
    REQUIRE(cp != nullptr);
    CHECK(cp->levels().size() == 3);
    CHECK(cp->lineColor() == QColor(Qt::red));
    CHECK_THAT(cp->lineWidth(), WithinAbs(2.0, 1e-9));
    CHECK(cp->labelsVisible());
    CHECK(cp->name() == "renamed");
    CHECK_FALSE(cp->isVisible());
}

TEST_CASE("ChangeContourPropertiesCommand: undo restores old properties",
          "[core][contour_command]") {
    ContourFixture fix;
    auto* cp = dynamic_cast<ContourPlot*>(fix.scene->itemAt(0));
    REQUIRE(cp != nullptr);
    QColor origColor = cp->lineColor();
    double origWidth = cp->lineWidth();
    auto cmd = std::make_unique<ChangeContourPropertiesCommand>(
        fix.scene.get(), 0,
        std::vector<double>{3.0, 7.0}, 0,
        QColor(Qt::blue), 3.0, true,
        "changed", false);
    cmd->execute();
    CHECK(cp->lineColor() == QColor(Qt::blue));
    cmd->undo();
    CHECK(cp->lineColor() == origColor);
    CHECK_THAT(cp->lineWidth(), WithinAbs(origWidth, 1e-9));
    CHECK_FALSE(cp->labelsVisible());
    CHECK(cp->isVisible());
}

TEST_CASE("ChangeContourPropertiesCommand: round-trip through CommandBus",
          "[core][contour_command]") {
    ContourFixture fix;
    CommandBus bus;
    QColor origColor = dynamic_cast<ContourPlot*>(fix.scene->itemAt(0))->lineColor();
    bus.execute(std::make_unique<ChangeContourPropertiesCommand>(
        fix.scene.get(), 0,
        std::vector<double>{1.0, 4.0, 9.0}, 0,
        QColor(Qt::cyan), 1.5, false,
        "bus_test", false));
    auto* cp = dynamic_cast<ContourPlot*>(fix.scene->itemAt(0));
    REQUIRE(cp != nullptr);
    CHECK(cp->lineColor() == QColor(Qt::cyan));
    bus.undo();
    CHECK(cp->lineColor() == origColor);
    bus.redo();
    CHECK(cp->lineColor() == QColor(Qt::cyan));
}

TEST_CASE("ChangeContourPropertiesCommand: description is non-empty",
          "[core][contour_command]") {
    ContourFixture fix;
    auto cmd = std::make_unique<ChangeContourPropertiesCommand>(
        fix.scene.get(), 0,
        std::vector<double>{}, 5,
        Qt::black, 1.0, false,
        "contour1", true);
    CHECK_FALSE(cmd->description().isEmpty());
}
