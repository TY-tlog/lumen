#include <catch2/catch_test_macros.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeHistogramPropertiesCommand.h"
#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/HistogramSeries.h"
#include "plot/PlotScene.h"

#include <memory>
#include <vector>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeHistogramPropertiesCommand;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::HistogramSeries;
using lumen::plot::PlotScene;

namespace {

struct HistFixture {
    std::shared_ptr<Rank1Dataset> ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(),
        std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
    std::unique_ptr<PlotScene> scene = std::make_unique<PlotScene>();

    HistFixture() {
        scene->addItem(std::make_unique<HistogramSeries>(ds));
    }
};

}  // namespace

TEST_CASE("ChangeHistogramPropertiesCommand: execute applies new properties",
          "[core][histogram_command]") {
    HistFixture fix;
    // binCount=0 triggers autoBinning with the given rule.
    auto cmd = std::make_unique<ChangeHistogramPropertiesCommand>(
        fix.scene.get(), 0, 0,
        HistogramSeries::BinRule::Scott,
        HistogramSeries::Normalization::Density,
        QColor(Qt::green), "renamed", false);
    cmd->execute();
    auto* hist = dynamic_cast<HistogramSeries*>(fix.scene->itemAt(0));
    REQUIRE(hist != nullptr);
    CHECK(hist->binRule() == HistogramSeries::BinRule::Scott);
    CHECK(hist->normalization() == HistogramSeries::Normalization::Density);
    CHECK(hist->fillColor() == QColor(Qt::green));
    CHECK(hist->name() == "renamed");
    CHECK_FALSE(hist->isVisible());
}

TEST_CASE("ChangeHistogramPropertiesCommand: undo restores old properties",
          "[core][histogram_command]") {
    HistFixture fix;
    auto* hist = dynamic_cast<HistogramSeries*>(fix.scene->itemAt(0));
    REQUIRE(hist != nullptr);
    QColor origColor = hist->fillColor();
    auto cmd = std::make_unique<ChangeHistogramPropertiesCommand>(
        fix.scene.get(), 0, 20,
        HistogramSeries::BinRule::FreedmanDiaconis,
        HistogramSeries::Normalization::Probability,
        QColor(Qt::red), "changed", false);
    cmd->execute();
    CHECK(hist->fillColor() == QColor(Qt::red));
    cmd->undo();
    CHECK(hist->fillColor() == origColor);
    CHECK(hist->binRule() == HistogramSeries::BinRule::Sturges);
    CHECK(hist->normalization() == HistogramSeries::Normalization::Count);
    CHECK(hist->isVisible());
}

TEST_CASE("ChangeHistogramPropertiesCommand: round-trip through CommandBus",
          "[core][histogram_command]") {
    HistFixture fix;
    CommandBus bus;
    QColor origColor = dynamic_cast<HistogramSeries*>(fix.scene->itemAt(0))->fillColor();
    bus.execute(std::make_unique<ChangeHistogramPropertiesCommand>(
        fix.scene.get(), 0, 15,
        HistogramSeries::BinRule::Scott,
        HistogramSeries::Normalization::Density,
        QColor(Qt::cyan), "bus_test", false));
    auto* hist = dynamic_cast<HistogramSeries*>(fix.scene->itemAt(0));
    REQUIRE(hist != nullptr);
    CHECK(hist->fillColor() == QColor(Qt::cyan));
    bus.undo();
    CHECK(hist->fillColor() == origColor);
    bus.redo();
    CHECK(hist->fillColor() == QColor(Qt::cyan));
}

TEST_CASE("ChangeHistogramPropertiesCommand: description is non-empty",
          "[core][histogram_command]") {
    HistFixture fix;
    auto cmd = std::make_unique<ChangeHistogramPropertiesCommand>(
        fix.scene.get(), 0, 0,
        HistogramSeries::BinRule::Sturges,
        HistogramSeries::Normalization::Count,
        QColor(70, 130, 180), "hist1", true);
    CHECK_FALSE(cmd->description().isEmpty());
}
