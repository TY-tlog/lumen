#include <catch2/catch_test_macros.hpp>

#include <dashboard/Dashboard.h>
#include <dashboard/DashboardSerializer.h>
#include <dashboard/PanelConfig.h>
#include <style/types.h>

#include <QJsonArray>
#include <QJsonDocument>

using namespace lumen::dashboard;
using namespace lumen::style;

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

TEST_CASE("DashboardSerializer: roundtrip preserves grid size", "[dashboard][persistence]")
{
    Dashboard db;
    db.setGridSize(3, 4);
    db.addPanel(pc(0, 0));

    QJsonObject json = DashboardSerializer::toJson(db);

    Dashboard db2;
    DashboardSerializer::fromJson(json, db2);

    REQUIRE(db2.gridRows() == 3);
    REQUIRE(db2.gridCols() == 4);
}

TEST_CASE("DashboardSerializer: roundtrip preserves panels", "[dashboard][persistence]")
{
    Dashboard db;
    db.setGridSize(2, 2);
    db.addPanel(pc(0, 0, QStringLiteral("Voltage")));
    db.addPanel(pc(0, 1, QStringLiteral("Current")));
    db.addPanel(pc(1, 0, QStringLiteral("Capacitance")));

    QJsonObject json = DashboardSerializer::toJson(db);

    Dashboard db2;
    DashboardSerializer::fromJson(json, db2);

    REQUIRE(db2.panelCount() == 3);
    REQUIRE(db2.panelConfigAt(0).title == "Voltage");
    REQUIRE(db2.panelConfigAt(1).title == "Current");
    REQUIRE(db2.panelConfigAt(2).row == 1);
    REQUIRE(db2.panelConfigAt(2).col == 0);
}

TEST_CASE("DashboardSerializer: roundtrip preserves panel spanning", "[dashboard][persistence]")
{
    Dashboard db;
    db.setGridSize(2, 3);
    PanelConfig wide;
    wide.row = 0;
    wide.col = 0;
    wide.rowSpan = 1;
    wide.colSpan = 2;
    db.addPanel(wide);

    QJsonObject json = DashboardSerializer::toJson(db);

    Dashboard db2;
    DashboardSerializer::fromJson(json, db2);

    REQUIRE(db2.panelConfigAt(0).colSpan == 2);
}

TEST_CASE("DashboardSerializer: roundtrip preserves linkGroup", "[dashboard][persistence]")
{
    Dashboard db;
    PanelConfig cfg;
    cfg.linkGroup = QStringLiteral("group-B");
    db.addPanel(cfg);

    QJsonObject json = DashboardSerializer::toJson(db);

    Dashboard db2;
    DashboardSerializer::fromJson(json, db2);

    REQUIRE(db2.panelConfigAt(0).linkGroup == "group-B");
}

TEST_CASE("DashboardSerializer: roundtrip preserves dashboard style", "[dashboard][persistence]")
{
    Dashboard db;
    db.addPanel(pc());

    Style s;
    s.backgroundColor = QColor(30, 30, 30);
    s.lineWidth = 2.0;
    db.setDashboardStyle(s);

    QJsonObject json = DashboardSerializer::toJson(db);

    Dashboard db2;
    DashboardSerializer::fromJson(json, db2);

    REQUIRE(db2.dashboardStyle().backgroundColor.has_value());
    REQUIRE(db2.dashboardStyle().lineWidth.has_value());
    REQUIRE(db2.dashboardStyle().lineWidth.value() == 2.0);
}

TEST_CASE("DashboardSerializer: roundtrip preserves panel style", "[dashboard][persistence]")
{
    Dashboard db;
    db.addPanel(pc());

    Style ps;
    ps.backgroundColor = QColor(Qt::red);
    db.setPanelStyle(0, ps);

    QJsonObject json = DashboardSerializer::toJson(db);

    Dashboard db2;
    DashboardSerializer::fromJson(json, db2);

    REQUIRE(db2.panelStyle(0).backgroundColor.has_value());
}

TEST_CASE("DashboardSerializer: empty dashboard serializes", "[dashboard][persistence]")
{
    Dashboard db;
    QJsonObject json = DashboardSerializer::toJson(db);
    REQUIRE(json.contains(QLatin1String("gridRows")));
    REQUIRE(json.contains(QLatin1String("panels")));
}

TEST_CASE("DashboardSerializer: migrateV1toV2 wraps plot in 1x1 dashboard", "[dashboard][persistence]")
{
    QJsonObject v1;
    v1[QLatin1String("version")] = 1;
    QJsonObject plot;
    plot[QLatin1String("title")] = QStringLiteral("Test");
    v1[QLatin1String("plot")] = plot;

    QJsonObject v2 = DashboardSerializer::migrateV1toV2(v1);

    REQUIRE(v2[QLatin1String("version")].toInt() == 2);
    REQUIRE(v2.contains(QLatin1String("dashboard")));

    QJsonObject db = v2[QLatin1String("dashboard")].toObject();
    REQUIRE(db[QLatin1String("gridRows")].toInt() == 1);
    REQUIRE(db[QLatin1String("gridCols")].toInt() == 1);

    QJsonArray panels = db[QLatin1String("panels")].toArray();
    REQUIRE(panels.size() == 1);

    QJsonObject panel = panels[0].toObject();
    REQUIRE(panel.contains(QLatin1String("plot")));
    REQUIRE(panel[QLatin1String("plot")].toObject()[QLatin1String("title")].toString() == "Test");
}

TEST_CASE("DashboardSerializer: migrateV1toV2 preserves original plot data", "[dashboard][persistence]")
{
    QJsonObject v1;
    v1[QLatin1String("version")] = 1;
    QJsonObject plot;
    QJsonObject viewport;
    viewport[QLatin1String("xmin")] = 0.0;
    viewport[QLatin1String("xmax")] = 100.0;
    plot[QLatin1String("viewport")] = viewport;
    v1[QLatin1String("plot")] = plot;

    QJsonObject v2 = DashboardSerializer::migrateV1toV2(v1);
    QJsonObject panel = v2[QLatin1String("dashboard")].toObject()
                           [QLatin1String("panels")].toArray()[0].toObject();
    QJsonObject migratedPlot = panel[QLatin1String("plot")].toObject();

    REQUIRE(migratedPlot.contains(QLatin1String("viewport")));
    REQUIRE(migratedPlot[QLatin1String("viewport")].toObject()
                [QLatin1String("xmax")].toDouble() == 100.0);
}

TEST_CASE("DashboardSerializer: fromJson with empty panels array is valid", "[dashboard][persistence]")
{
    QJsonObject obj;
    obj[QLatin1String("gridRows")] = 2;
    obj[QLatin1String("gridCols")] = 2;
    obj[QLatin1String("panels")] = QJsonArray();

    Dashboard db;
    DashboardSerializer::fromJson(obj, db);

    REQUIRE(db.gridRows() == 2);
    REQUIRE(db.panelCount() == 0);
}

TEST_CASE("DashboardSerializer: JSON output is valid JSON", "[dashboard][persistence]")
{
    Dashboard db;
    db.setGridSize(2, 2);
    db.addPanel(pc(0, 0, QStringLiteral("A")));
    db.addPanel(pc(1, 1, QStringLiteral("B")));

    QJsonObject json = DashboardSerializer::toJson(db);
    QJsonDocument doc(json);
    QByteArray bytes = doc.toJson();

    QJsonParseError err;
    QJsonDocument reparsed = QJsonDocument::fromJson(bytes, &err);
    REQUIRE(err.error == QJsonParseError::NoError);
    REQUIRE(reparsed.isObject());
}
