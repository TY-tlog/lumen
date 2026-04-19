#include <catch2/catch_test_macros.hpp>
#include "pixel_utils.h"

#include <dashboard/Dashboard.h>
#include <dashboard/DashboardSerializer.h>
#include <dashboard/DashboardWidget.h>
#include <dashboard/LinkGroup.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>
#include <style/StyleManager.h>
#include <ui/PlotCanvas.h>

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>

#include <QtGlobal>
#include <cmath>
#include <memory>

static void initResources()
{
    Q_INIT_RESOURCE(styles);
    Q_INIT_RESOURCE(fonts);
}

namespace {

QApplication* ensureApp()
{
    if (QApplication::instance() == nullptr) {
        static int argc = 1;
        static const char* argv[] = {"smoke_dashboard"};
        static auto* app =
            new QApplication(argc, const_cast<char**>(argv));
        initResources();
        lumen::StyleManager sm;
        sm.apply();
        return app;
    }
    return static_cast<QApplication*>(QApplication::instance());
}

void addSineToScene(lumen::plot::PlotScene* scene)
{
    std::vector<double> xd, yd;
    for (int i = 0; i < 100; ++i) {
        double x = static_cast<double>(i) * 0.1;
        xd.push_back(x);
        yd.push_back(std::sin(x));
    }
    auto xDs = std::make_shared<lumen::data::Rank1Dataset>(
        "x", lumen::data::Unit::dimensionless(), std::move(xd));
    auto yDs = std::make_shared<lumen::data::Rank1Dataset>(
        "y", lumen::data::Unit::dimensionless(), std::move(yd));

    lumen::plot::PlotStyle style;
    style.color = QColor(Qt::blue);
    auto series = std::make_unique<lumen::plot::LineSeries>(
        xDs, yDs, style, QStringLiteral("sine"));
    scene->addItem(std::move(series));
    scene->autoRange();
}

}  // namespace

TEST_CASE("S6: dashboard widget creates and shows panels", "[smoke][dashboard][S6]")
{
    ensureApp();

    lumen::dashboard::Dashboard db;
    db.setGridSize(1, 2);

    lumen::dashboard::PanelConfig cfg0;
    cfg0.row = 0;
    cfg0.col = 0;
    db.addPanel(cfg0);

    lumen::dashboard::PanelConfig cfg1;
    cfg1.row = 0;
    cfg1.col = 1;
    db.addPanel(cfg1);

    lumen::dashboard::DashboardWidget widget(&db);
    widget.setAttribute(Qt::WA_DontShowOnScreen);
    widget.resize(800, 400);
    widget.show();

    REQUIRE(widget.canvasAt(0) != nullptr);
    REQUIRE(widget.canvasAt(1) != nullptr);
}

TEST_CASE("S7: dashboard panels with data have visual content", "[smoke][dashboard][S7]")
{
    ensureApp();

    lumen::dashboard::Dashboard db;
    db.setGridSize(1, 2);

    lumen::dashboard::PanelConfig cfg0;
    cfg0.row = 0;
    cfg0.col = 0;
    db.addPanel(cfg0);

    lumen::dashboard::PanelConfig cfg1;
    cfg1.row = 0;
    cfg1.col = 1;
    db.addPanel(cfg1);

    addSineToScene(db.sceneAt(0));
    addSineToScene(db.sceneAt(1));

    lumen::dashboard::DashboardWidget widget(&db);
    widget.setAttribute(Qt::WA_DontShowOnScreen);
    widget.resize(800, 400);
    widget.show();

    QPixmap grab0 = widget.canvasAt(0)->grab();
    QPixmap grab1 = widget.canvasAt(1)->grab();

    REQUIRE(lumen::test::hasVisualContent(grab0.toImage()));
    REQUIRE(lumen::test::hasVisualContent(grab1.toImage()));
}

TEST_CASE("S8: link group propagates X range", "[smoke][dashboard][S8]")
{
    lumen::dashboard::LinkGroup lg(QStringLiteral("default"));
    lg.addPanel(0);
    lg.addPanel(1);

    double receivedMin = 0, receivedMax = 0;
    int receivedSource = -1;
    QObject::connect(&lg, &lumen::dashboard::LinkGroup::xRangeChanged,
                     [&](int src, double mn, double mx) {
        receivedSource = src;
        receivedMin = mn;
        receivedMax = mx;
    });

    lg.propagateXRange(0, 5.0, 15.0);

    REQUIRE(receivedSource == 0);
    REQUIRE(receivedMin == 5.0);
    REQUIRE(receivedMax == 15.0);
}

TEST_CASE("S9: crosshair sync propagates data X", "[smoke][dashboard][S9]")
{
    lumen::dashboard::LinkGroup lg(QStringLiteral("default"));
    lg.addPanel(0);
    lg.addPanel(1);

    double receivedX = -1;
    QObject::connect(&lg, &lumen::dashboard::LinkGroup::crosshairMoved,
                     [&](int, double x) { receivedX = x; });

    lg.propagateCrosshair(0, 42.0);

    REQUIRE(receivedX == 42.0);
}

TEST_CASE("S10: dashboard style propagates to cascade", "[smoke][dashboard][S10]")
{
    lumen::dashboard::Dashboard db;
    lumen::style::Style s;
    s.backgroundColor = QColor(40, 40, 40);
    db.setDashboardStyle(s);

    REQUIRE(db.dashboardStyle().backgroundColor.has_value());
    REQUIRE(db.dashboardStyle().backgroundColor->red() == 40);
}

TEST_CASE("S11: dashboard save/load roundtrip", "[smoke][dashboard][S11]")
{
    lumen::dashboard::Dashboard db;
    db.setGridSize(2, 3);

    lumen::dashboard::PanelConfig cfg;
    cfg.row = 0;
    cfg.col = 0;
    cfg.title = QStringLiteral("Voltage");
    db.addPanel(cfg);

    cfg.row = 1;
    cfg.col = 2;
    cfg.title = QStringLiteral("Current");
    db.addPanel(cfg);

    QJsonObject json = lumen::dashboard::DashboardSerializer::toJson(db);

    lumen::dashboard::Dashboard db2;
    lumen::dashboard::DashboardSerializer::fromJson(json, db2);

    REQUIRE(db2.gridRows() == 2);
    REQUIRE(db2.gridCols() == 3);
    REQUIRE(db2.panelCount() == 2);
    REQUIRE(db2.panelConfigAt(0).title == "Voltage");
    REQUIRE(db2.panelConfigAt(1).title == "Current");
}

TEST_CASE("S12: V1 workspace migrates to dashboard", "[smoke][dashboard][S12]")
{
    QJsonObject v1;
    v1[QLatin1String("version")] = 1;
    QJsonObject plot;
    plot[QLatin1String("title")] = QStringLiteral("Migrated");
    v1[QLatin1String("plot")] = plot;

    QJsonObject v2 = lumen::dashboard::DashboardSerializer::migrateV1toV2(v1);

    REQUIRE(v2[QLatin1String("version")].toInt() == 2);
    REQUIRE(v2.contains(QLatin1String("dashboard")));
}
