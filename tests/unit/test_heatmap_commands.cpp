#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeHeatmapPropertiesCommand.h"
#include "data/CoordinateArray.h"
#include "data/Dimension.h"
#include "data/Grid2D.h"
#include "data/Unit.h"
#include "plot/Colormap.h"
#include "plot/Heatmap.h"
#include "plot/PlotScene.h"

#include <memory>
#include <vector>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeHeatmapPropertiesCommand;
using lumen::data::CoordinateArray;
using lumen::data::Dimension;
using lumen::data::Grid2D;
using lumen::data::Unit;
using lumen::plot::Colormap;
using lumen::plot::Heatmap;
using lumen::plot::PlotScene;
using Catch::Matchers::WithinAbs;

namespace {

std::shared_ptr<Grid2D> makeTestGrid()
{
    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), 3,
                   CoordinateArray(0.0, 1.0, 3)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), 2,
                   CoordinateArray(0.0, 1.0, 2)};
    return std::make_shared<Grid2D>(
        QStringLiteral("test"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY),
        std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
}

struct HeatFixture {
    std::shared_ptr<Grid2D> grid = makeTestGrid();
    std::unique_ptr<PlotScene> scene = std::make_unique<PlotScene>();

    HeatFixture() {
        scene->addItem(std::make_unique<Heatmap>(
            grid, Colormap::builtin(Colormap::Builtin::Viridis)));
    }
};

}  // namespace

TEST_CASE("ChangeHeatmapPropertiesCommand: execute applies new properties",
          "[core][heatmap_command]") {
    HeatFixture fix;
    auto newCmap = Colormap::builtin(Colormap::Builtin::Inferno);
    auto cmd = std::make_unique<ChangeHeatmapPropertiesCommand>(
        fix.scene.get(), 0,
        newCmap, 0.0, 10.0, false,
        Heatmap::Interpolation::Bilinear, 0.5,
        "renamed", false);
    cmd->execute();
    auto* hm = dynamic_cast<Heatmap*>(fix.scene->itemAt(0));
    REQUIRE(hm != nullptr);
    CHECK(hm->interpolation() == Heatmap::Interpolation::Bilinear);
    CHECK_THAT(hm->opacity(), WithinAbs(0.5, 1e-9));
    CHECK(hm->name() == "renamed");
    CHECK_FALSE(hm->isVisible());
}

TEST_CASE("ChangeHeatmapPropertiesCommand: undo restores old properties",
          "[core][heatmap_command]") {
    HeatFixture fix;
    auto* hm = dynamic_cast<Heatmap*>(fix.scene->itemAt(0));
    REQUIRE(hm != nullptr);
    auto origInterp = hm->interpolation();
    auto cmd = std::make_unique<ChangeHeatmapPropertiesCommand>(
        fix.scene.get(), 0,
        Colormap::builtin(Colormap::Builtin::Plasma),
        -1.0, 2.0, false,
        Heatmap::Interpolation::Bilinear, 0.3,
        "changed", false);
    cmd->execute();
    CHECK(hm->interpolation() == Heatmap::Interpolation::Bilinear);
    cmd->undo();
    CHECK(hm->interpolation() == origInterp);
    CHECK_THAT(hm->opacity(), WithinAbs(1.0, 1e-9));
    CHECK(hm->isVisible());
}

TEST_CASE("ChangeHeatmapPropertiesCommand: round-trip through CommandBus",
          "[core][heatmap_command]") {
    HeatFixture fix;
    CommandBus bus;
    auto origInterp = dynamic_cast<Heatmap*>(fix.scene->itemAt(0))->interpolation();
    bus.execute(std::make_unique<ChangeHeatmapPropertiesCommand>(
        fix.scene.get(), 0,
        Colormap::builtin(Colormap::Builtin::Magma),
        0.0, 5.0, false,
        Heatmap::Interpolation::Bilinear, 0.8,
        "bus_test", false));
    auto* hm = dynamic_cast<Heatmap*>(fix.scene->itemAt(0));
    REQUIRE(hm != nullptr);
    CHECK(hm->interpolation() == Heatmap::Interpolation::Bilinear);
    bus.undo();
    CHECK(hm->interpolation() == origInterp);
    bus.redo();
    CHECK(hm->interpolation() == Heatmap::Interpolation::Bilinear);
}

TEST_CASE("ChangeHeatmapPropertiesCommand: description is non-empty",
          "[core][heatmap_command]") {
    HeatFixture fix;
    auto cmd = std::make_unique<ChangeHeatmapPropertiesCommand>(
        fix.scene.get(), 0,
        Colormap::builtin(Colormap::Builtin::Viridis),
        0.0, 1.0, true,
        Heatmap::Interpolation::Nearest, 1.0,
        "heat1", true);
    CHECK_FALSE(cmd->description().isEmpty());
}
