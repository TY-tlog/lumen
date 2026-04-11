#include <catch2/catch_test_macros.hpp>

#include "core/Command.h"
#include "core/CommandBus.h"
#include "core/commands/ChangeLineStyleCommand.h"
#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include <memory>
#include "plot/LineSeries.h"
#include "plot/PlotScene.h"
#include "plot/PlotStyle.h"

#include <QSignalSpy>

#include <memory>
#include <vector>

using lumen::core::Command;
using lumen::core::CommandBus;
using lumen::core::commands::ChangeLineStyleCommand;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::LineSeries;
using lumen::plot::PlotScene;
using lumen::plot::PlotStyle;

namespace {

/// A trivial command that increments/decrements an integer.
class IncrementCommand : public Command {
public:
    explicit IncrementCommand(int* target, int delta = 1)
        : target_(target), delta_(delta) {}

    void execute() override { *target_ += delta_; }
    void undo() override { *target_ -= delta_; }
    QString description() const override {
        return QStringLiteral("Increment by %1").arg(delta_);
    }

private:
    int* target_;
    int delta_;
};

}  // namespace

TEST_CASE("CommandBus: execute pushes to undo stack", "[core][command_bus]") {
    CommandBus bus;
    int value = 0;

    REQUIRE_FALSE(bus.canUndo());
    REQUIRE_FALSE(bus.canRedo());

    bus.execute(std::make_unique<IncrementCommand>(&value));

    CHECK(value == 1);
    CHECK(bus.canUndo());
    CHECK_FALSE(bus.canRedo());
    CHECK(bus.undoDescription() == "Increment by 1");
}

TEST_CASE("CommandBus: undo reverses the command", "[core][command_bus]") {
    CommandBus bus;
    int value = 0;

    bus.execute(std::make_unique<IncrementCommand>(&value));
    CHECK(value == 1);

    bus.undo();

    CHECK(value == 0);
    CHECK_FALSE(bus.canUndo());
    CHECK(bus.canRedo());
    CHECK(bus.redoDescription() == "Increment by 1");
}

TEST_CASE("CommandBus: redo re-applies the command", "[core][command_bus]") {
    CommandBus bus;
    int value = 0;

    bus.execute(std::make_unique<IncrementCommand>(&value));
    bus.undo();
    CHECK(value == 0);

    bus.redo();

    CHECK(value == 1);
    CHECK(bus.canUndo());
    CHECK_FALSE(bus.canRedo());
}

TEST_CASE("CommandBus: new execute clears redo stack", "[core][command_bus]") {
    CommandBus bus;
    int value = 0;

    bus.execute(std::make_unique<IncrementCommand>(&value, 10));
    bus.undo();
    CHECK(bus.canRedo());

    // Execute a new command — redo stack should be cleared.
    bus.execute(std::make_unique<IncrementCommand>(&value, 5));
    CHECK(value == 5);
    CHECK_FALSE(bus.canRedo());
    CHECK(bus.canUndo());
}

TEST_CASE("CommandBus: signals are emitted", "[core][command_bus]") {
    CommandBus bus;
    int value = 0;

    QSignalSpy executedSpy(&bus, &CommandBus::commandExecuted);
    QSignalSpy stateChangedSpy(&bus, &CommandBus::undoRedoStateChanged);

    REQUIRE(executedSpy.isValid());
    REQUIRE(stateChangedSpy.isValid());

    bus.execute(std::make_unique<IncrementCommand>(&value));

    CHECK(executedSpy.count() == 1);
    CHECK(executedSpy.at(0).at(0).toString() == "Increment by 1");
    CHECK(stateChangedSpy.count() == 1);

    bus.undo();
    // undo emits undoRedoStateChanged but not commandExecuted.
    CHECK(executedSpy.count() == 1);
    CHECK(stateChangedSpy.count() == 2);

    bus.redo();
    CHECK(stateChangedSpy.count() == 3);
}

TEST_CASE("CommandBus: multiple undo/redo cycle", "[core][command_bus]") {
    CommandBus bus;
    int value = 0;

    bus.execute(std::make_unique<IncrementCommand>(&value, 1));
    bus.execute(std::make_unique<IncrementCommand>(&value, 2));
    bus.execute(std::make_unique<IncrementCommand>(&value, 3));
    CHECK(value == 6);

    bus.undo();
    CHECK(value == 3);
    bus.undo();
    CHECK(value == 1);
    bus.undo();
    CHECK(value == 0);
    CHECK_FALSE(bus.canUndo());

    bus.redo();
    CHECK(value == 1);
    bus.redo();
    CHECK(value == 3);
    bus.redo();
    CHECK(value == 6);
    CHECK_FALSE(bus.canRedo());
}

TEST_CASE("CommandBus: undo/redo on empty stacks is safe",
          "[core][command_bus]") {
    CommandBus bus;

    // Should not crash or throw.
    bus.undo();
    bus.redo();
    CHECK_FALSE(bus.canUndo());
    CHECK_FALSE(bus.canRedo());
    CHECK(bus.undoDescription().isEmpty());
    CHECK(bus.redoDescription().isEmpty());
}

TEST_CASE("ChangeLineStyleCommand: execute changes style, undo restores",
          "[core][command_bus][change_line_style]") {
    // Build a PlotScene with one series.
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{10.0, 20.0, 30.0});

    PlotStyle originalStyle;
    originalStyle.color = QColor(Qt::red);
    originalStyle.lineWidth = 1.5;
    originalStyle.penStyle = Qt::SolidLine;

    PlotScene scene;
    scene.addSeries(LineSeries(xCol, yCol, originalStyle, "Original"));

    // Verify initial state.
    CHECK(scene.seriesAt(0).style().color == QColor(Qt::red));
    CHECK(scene.seriesAt(0).name() == "Original");
    CHECK(scene.seriesAt(0).isVisible());

    // Create and execute command.
    PlotStyle newStyle;
    newStyle.color = QColor(Qt::blue);
    newStyle.lineWidth = 3.0;
    newStyle.penStyle = Qt::DashLine;

    auto cmd = std::make_unique<ChangeLineStyleCommand>(
        &scene, 0, newStyle, "Renamed", false);

    cmd->execute();

    CHECK(scene.seriesAt(0).style().color == QColor(Qt::blue));
    CHECK(scene.seriesAt(0).style().lineWidth == 3.0);
    CHECK(scene.seriesAt(0).style().penStyle == Qt::DashLine);
    CHECK(scene.seriesAt(0).name() == "Renamed");
    CHECK_FALSE(scene.seriesAt(0).isVisible());

    // Undo restores original.
    cmd->undo();

    CHECK(scene.seriesAt(0).style().color == QColor(Qt::red));
    CHECK(scene.seriesAt(0).style().lineWidth == 1.5);
    CHECK(scene.seriesAt(0).style().penStyle == Qt::SolidLine);
    CHECK(scene.seriesAt(0).name() == "Original");
    CHECK(scene.seriesAt(0).isVisible());
}

TEST_CASE("ChangeLineStyleCommand: round-trip through CommandBus",
          "[core][command_bus][change_line_style]") {
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 2.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{10.0, 20.0});

    PlotScene scene;
    scene.addSeries(LineSeries(xCol, yCol,
                               PlotStyle::fromPalette(0), "SeriesA"));

    CommandBus bus;
    PlotStyle newStyle;
    newStyle.color = QColor(Qt::green);
    newStyle.lineWidth = 2.0;

    bus.execute(std::make_unique<ChangeLineStyleCommand>(
        &scene, 0, newStyle, "SeriesB", true));

    CHECK(scene.seriesAt(0).name() == "SeriesB");
    CHECK(scene.seriesAt(0).style().color == QColor(Qt::green));

    bus.undo();
    CHECK(scene.seriesAt(0).name() == "SeriesA");

    bus.redo();
    CHECK(scene.seriesAt(0).name() == "SeriesB");
}
