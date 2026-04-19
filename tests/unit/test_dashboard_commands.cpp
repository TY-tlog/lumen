#include <catch2/catch_test_macros.hpp>

#include <core/commands/DashboardCommands.h>
#include <dashboard/Dashboard.h>

using namespace lumen::core::commands;
using namespace lumen::dashboard;

namespace {
PanelConfig pc(int row = 0, int col = 0, const QString& title = {})
{
    PanelConfig c;
    c.row = row;
    c.col = col;
    c.title = title;
    return c;
}
}  // namespace

TEST_CASE("AddDashboardPanelCommand: execute adds panel", "[dashboard][commands]")
{
    Dashboard db;
    db.setGridSize(2, 2);

    AddDashboardPanelCommand cmd(&db, pc(0, 0, QStringLiteral("V")));
    cmd.execute();

    REQUIRE(db.panelCount() == 1);
    REQUIRE(db.panelConfigAt(0).title == "V");
}

TEST_CASE("AddDashboardPanelCommand: undo removes panel", "[dashboard][commands]")
{
    Dashboard db;
    db.setGridSize(2, 2);

    AddDashboardPanelCommand cmd(&db, pc(0, 0));
    cmd.execute();
    REQUIRE(db.panelCount() == 1);

    cmd.undo();
    REQUIRE(db.panelCount() == 0);
}

TEST_CASE("AddDashboardPanelCommand: description is non-empty", "[dashboard][commands]")
{
    Dashboard db;
    AddDashboardPanelCommand cmd(&db, pc());
    REQUIRE(!cmd.description().isEmpty());
}

TEST_CASE("RemoveDashboardPanelCommand: execute removes panel", "[dashboard][commands]")
{
    Dashboard db;
    db.addPanel(pc(0, 0, QStringLiteral("A")));
    db.addPanel(pc(0, 1, QStringLiteral("B")));

    RemoveDashboardPanelCommand cmd(&db, 0);
    cmd.execute();

    REQUIRE(db.panelCount() == 1);
    REQUIRE(db.panelConfigAt(0).title == "B");
}

TEST_CASE("RemoveDashboardPanelCommand: undo restores panel", "[dashboard][commands]")
{
    Dashboard db;
    db.addPanel(pc(0, 0, QStringLiteral("X")));

    RemoveDashboardPanelCommand cmd(&db, 0);
    cmd.execute();
    REQUIRE(db.panelCount() == 0);

    cmd.undo();
    REQUIRE(db.panelCount() == 1);
    REQUIRE(db.panelConfigAt(0).title == "X");
}

TEST_CASE("ChangeDashboardLayoutCommand: execute changes grid", "[dashboard][commands]")
{
    Dashboard db;
    db.setGridSize(1, 1);

    ChangeDashboardLayoutCommand cmd(&db, 3, 4);
    cmd.execute();

    REQUIRE(db.gridRows() == 3);
    REQUIRE(db.gridCols() == 4);
}

TEST_CASE("ChangeDashboardLayoutCommand: undo restores grid", "[dashboard][commands]")
{
    Dashboard db;
    db.setGridSize(2, 2);

    ChangeDashboardLayoutCommand cmd(&db, 4, 4);
    cmd.execute();
    REQUIRE(db.gridRows() == 4);

    cmd.undo();
    REQUIRE(db.gridRows() == 2);
    REQUIRE(db.gridCols() == 2);
}

TEST_CASE("ChangeDashboardLayoutCommand: description is non-empty", "[dashboard][commands]")
{
    Dashboard db;
    ChangeDashboardLayoutCommand cmd(&db, 2, 2);
    REQUIRE(!cmd.description().isEmpty());
}
