#include <catch2/catch_test_macros.hpp>

#include <dashboard/Dashboard.h>
#include <dashboard/PanelConfig.h>
#include <plot/PlotScene.h>

#include <QSignalSpy>

using namespace lumen::dashboard;

namespace {
PanelConfig pc(int row = 0, int col = 0)
{
    PanelConfig c;
    c.row = row;
    c.col = col;
    return c;
}
}  // namespace

TEST_CASE("Dashboard: default state", "[dashboard]")
{
    Dashboard db;
    REQUIRE(db.panelCount() == 0);
    REQUIRE(db.gridRows() == 1);
    REQUIRE(db.gridCols() == 1);
}

TEST_CASE("Dashboard: addPanel returns sequential indices", "[dashboard]")
{
    Dashboard db;
    REQUIRE(db.addPanel(pc(0, 0)) == 0);
    REQUIRE(db.addPanel(pc(0, 1)) == 1);
    REQUIRE(db.addPanel(pc(1, 0)) == 2);
    REQUIRE(db.panelCount() == 3);
}

TEST_CASE("Dashboard: addPanel emits signal", "[dashboard]")
{
    Dashboard db;
    QSignalSpy spy(&db, &Dashboard::panelAdded);
    db.addPanel(pc(0, 0));
    REQUIRE(spy.count() == 1);
    REQUIRE(spy.at(0).at(0).toInt() == 0);
}

TEST_CASE("Dashboard: removePanel reduces count", "[dashboard]")
{
    Dashboard db;
    db.addPanel(pc(0, 0));
    db.addPanel(pc(0, 1));
    db.removePanel(0);
    REQUIRE(db.panelCount() == 1);
}

TEST_CASE("Dashboard: removePanel emits signal", "[dashboard]")
{
    Dashboard db;
    db.addPanel(pc(0, 0));
    QSignalSpy spy(&db, &Dashboard::panelRemoved);
    db.removePanel(0);
    REQUIRE(spy.count() == 1);
}

TEST_CASE("Dashboard: removePanel out of range is no-op", "[dashboard]")
{
    Dashboard db;
    db.addPanel(pc(0, 0));
    db.removePanel(-1);
    db.removePanel(5);
    REQUIRE(db.panelCount() == 1);
}

TEST_CASE("Dashboard: panelConfigAt returns correct config", "[dashboard]")
{
    Dashboard db;
    PanelConfig cfg;
    cfg.row = 1;
    cfg.col = 2;
    cfg.title = QStringLiteral("Voltage");
    db.addPanel(cfg);

    REQUIRE(db.panelConfigAt(0).row == 1);
    REQUIRE(db.panelConfigAt(0).col == 2);
    REQUIRE(db.panelConfigAt(0).title == "Voltage");
}

TEST_CASE("Dashboard: sceneAt returns valid PlotScene", "[dashboard]")
{
    Dashboard db;
    db.addPanel(pc(0, 0));
    auto* scene = db.sceneAt(0);
    REQUIRE(scene != nullptr);
}

TEST_CASE("Dashboard: sceneAt out of range returns nullptr", "[dashboard]")
{
    Dashboard db;
    REQUIRE(db.sceneAt(0) == nullptr);
    REQUIRE(db.sceneAt(-1) == nullptr);
}

TEST_CASE("Dashboard: setGridSize clamps to 1..8", "[dashboard]")
{
    Dashboard db;
    db.setGridSize(0, 0);
    REQUIRE(db.gridRows() == 1);
    REQUIRE(db.gridCols() == 1);

    db.setGridSize(10, 12);
    REQUIRE(db.gridRows() == 8);
    REQUIRE(db.gridCols() == 8);

    db.setGridSize(3, 4);
    REQUIRE(db.gridRows() == 3);
    REQUIRE(db.gridCols() == 4);
}

TEST_CASE("Dashboard: setGridSize emits layoutChanged", "[dashboard]")
{
    Dashboard db;
    QSignalSpy spy(&db, &Dashboard::layoutChanged);
    db.setGridSize(2, 3);
    REQUIRE(spy.count() == 1);
}

TEST_CASE("Dashboard: each panel has independent PlotScene", "[dashboard]")
{
    Dashboard db;
    db.addPanel(pc(0, 0));
    db.addPanel(pc(0, 1));
    REQUIRE(db.sceneAt(0) != db.sceneAt(1));
}

TEST_CASE("Dashboard: default linkGroup is default", "[dashboard]")
{
    Dashboard db;
    db.addPanel(pc());
    REQUIRE(db.panelConfigAt(0).linkGroup == "default");
}

TEST_CASE("Dashboard: panelConfig is mutable", "[dashboard]")
{
    Dashboard db;
    db.addPanel(pc(0, 0));
    db.panelConfigAt(0).title = QStringLiteral("Changed");
    REQUIRE(db.panelConfigAt(0).title == "Changed");
}
