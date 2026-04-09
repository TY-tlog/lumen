#include <catch2/catch_test_macros.hpp>

#include "core/CommandBus.h"
#include "core/commands/ChangeTitleCommand.h"
#include "plot/PlotScene.h"

#include <QFont>

#include <memory>

using lumen::core::CommandBus;
using lumen::core::commands::ChangeTitleCommand;
using lumen::plot::PlotScene;

TEST_CASE("ChangeTitleCommand: execute changes title text and font",
          "[core][title_command]") {
    PlotScene scene;
    scene.setTitle("Old Title");
    scene.setTitleFontPx(17);
    scene.setTitleWeight(QFont::DemiBold);

    auto cmd = std::make_unique<ChangeTitleCommand>(&scene, "New Title", 24,
                                                    QFont::Bold);

    cmd->execute();

    CHECK(scene.title() == "New Title");
    CHECK(scene.titleFontPx() == 24);
    CHECK(scene.titleWeight() == QFont::Bold);
}

TEST_CASE("ChangeTitleCommand: undo restores original title",
          "[core][title_command]") {
    PlotScene scene;
    scene.setTitle("Original");
    scene.setTitleFontPx(17);
    scene.setTitleWeight(QFont::DemiBold);

    auto cmd = std::make_unique<ChangeTitleCommand>(&scene, "Changed", 30,
                                                    QFont::Bold);

    cmd->execute();
    CHECK(scene.title() == "Changed");
    CHECK(scene.titleFontPx() == 30);
    CHECK(scene.titleWeight() == QFont::Bold);

    cmd->undo();
    CHECK(scene.title() == "Original");
    CHECK(scene.titleFontPx() == 17);
    CHECK(scene.titleWeight() == QFont::DemiBold);
}

TEST_CASE("ChangeTitleCommand: round-trip through CommandBus",
          "[core][title_command]") {
    PlotScene scene;
    scene.setTitle("Title A");

    CommandBus bus;

    bus.execute(
        std::make_unique<ChangeTitleCommand>(&scene, "Title B", 20, QFont::Normal));

    CHECK(scene.title() == "Title B");
    CHECK(scene.titleFontPx() == 20);
    CHECK(scene.titleWeight() == QFont::Normal);

    bus.undo();
    CHECK(scene.title() == "Title A");
    CHECK(scene.titleFontPx() == 17);
    CHECK(scene.titleWeight() == QFont::DemiBold);

    bus.redo();
    CHECK(scene.title() == "Title B");
}

TEST_CASE("ChangeTitleCommand: description is non-empty",
          "[core][title_command]") {
    PlotScene scene;

    auto cmd = std::make_unique<ChangeTitleCommand>(&scene, "Title", 17,
                                                    QFont::DemiBold);

    CHECK_FALSE(cmd->description().isEmpty());
}
