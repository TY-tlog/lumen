#include <catch2/catch_test_macros.hpp>

#include <dashboard/Dashboard.h>
#include <style/cascade.h>
#include <style/theme_registry.h>
#include <style/types.h>

#include <QSignalSpy>

using namespace lumen::dashboard;
using namespace lumen::style;

namespace {
PanelConfig pc(int row = 0, int col = 0)
{
    PanelConfig c;
    c.row = row;
    c.col = col;
    return c;
}
}  // namespace

TEST_CASE("Dashboard cascade: empty dashboard style passes through theme", "[dashboard][cascade]")
{
    ThemeRegistry reg;
    Style theme = reg.theme(QStringLiteral("lumen-light"));
    Style preset;  // empty dashboard style
    Style plot;
    Style element;

    Style resolved = cascade(theme, preset, plot, element);
    REQUIRE(resolved.backgroundColor == theme.backgroundColor);
}

TEST_CASE("Dashboard cascade: dashboard style overrides theme", "[dashboard][cascade]")
{
    Style theme;
    theme.backgroundColor = QColor(Qt::white);

    Style dashStyle;
    dashStyle.backgroundColor = QColor(50, 50, 50);

    Style resolved = cascade(theme, dashStyle, Style{}, Style{});
    REQUIRE(resolved.backgroundColor.has_value());
    REQUIRE(resolved.backgroundColor->red() == 50);
}

TEST_CASE("Dashboard cascade: panel style overrides dashboard", "[dashboard][cascade]")
{
    Style theme;
    theme.backgroundColor = QColor(Qt::white);

    Style dashStyle;
    dashStyle.backgroundColor = QColor(50, 50, 50);

    Style panelStyle;
    panelStyle.backgroundColor = QColor(Qt::red);

    Style resolved = cascade(theme, dashStyle, panelStyle, Style{});
    REQUIRE(resolved.backgroundColor.has_value());
    REQUIRE(resolved.backgroundColor.value() == QColor(Qt::red));
}

TEST_CASE("Dashboard cascade: element override wins over all", "[dashboard][cascade]")
{
    Style theme;
    theme.lineWidth = 1.0;

    Style dashStyle;
    dashStyle.lineWidth = 2.0;

    Style panelStyle;
    panelStyle.lineWidth = 3.0;

    Style element;
    element.lineWidth = 4.0;

    Style resolved = cascade(theme, dashStyle, panelStyle, element);
    REQUIRE(resolved.lineWidth.has_value());
    REQUIRE(resolved.lineWidth.value() == 4.0);
}

TEST_CASE("Dashboard: setDashboardStyle emits styleChanged", "[dashboard][cascade]")
{
    Dashboard db;
    QSignalSpy spy(&db, &Dashboard::styleChanged);

    Style s;
    s.backgroundColor = QColor(Qt::black);
    db.setDashboardStyle(s);

    REQUIRE(spy.count() == 1);
    REQUIRE(db.dashboardStyle().backgroundColor.has_value());
}

TEST_CASE("Dashboard: setPanelStyle stores per-panel style", "[dashboard][cascade]")
{
    Dashboard db;
    db.addPanel(pc(0, 0));
    db.addPanel(pc(0, 1));

    Style s;
    s.lineWidth = 5.0;
    db.setPanelStyle(0, s);

    REQUIRE(db.panelStyle(0).lineWidth.has_value());
    REQUIRE(db.panelStyle(0).lineWidth.value() == 5.0);
    REQUIRE_FALSE(db.panelStyle(1).lineWidth.has_value());
}

TEST_CASE("Dashboard: panelStyle out of range returns empty", "[dashboard][cascade]")
{
    Dashboard db;
    const auto& s = db.panelStyle(99);
    REQUIRE_FALSE(s.backgroundColor.has_value());
}

TEST_CASE("Dashboard cascade: standalone plot has empty preset", "[dashboard][cascade]")
{
    Style theme;
    theme.backgroundColor = QColor(Qt::white);

    Style resolved = cascade(theme, Style{}, Style{}, Style{});
    REQUIRE(resolved.backgroundColor.value() == QColor(Qt::white));
}

TEST_CASE("Dashboard cascade with trace shows preset source", "[dashboard][cascade]")
{
    Style theme;
    theme.backgroundColor = QColor(Qt::white);

    Style dashStyle;
    dashStyle.lineWidth = 2.5;

    CascadeTrace trace;
    cascadeWithTrace(theme, QStringLiteral("lumen-light"),
                     dashStyle, QStringLiteral("dashboard"),
                     Style{}, Style{}, trace);

    REQUIRE(!trace.isEmpty());

    bool foundPreset = false;
    for (int i = 0; i < trace.size(); ++i) {
        if (trace[i].property == QStringLiteral("lineWidth") &&
            trace[i].source == CascadeLevel::Preset) {
            foundPreset = true;
        }
    }
    REQUIRE(foundPreset);
}

TEST_CASE("Dashboard cascade: sub-styles merge correctly", "[dashboard][cascade]")
{
    Style theme;
    StrokeStyle ts;
    ts.color = QColor(Qt::black);
    ts.width = 1.0;
    theme.stroke = ts;

    Style dashStyle;
    StrokeStyle ds;
    ds.width = 2.5;
    dashStyle.stroke = ds;

    Style resolved = cascade(theme, dashStyle, Style{}, Style{});
    REQUIRE(resolved.stroke.has_value());
    REQUIRE(resolved.stroke->color.has_value());
    REQUIRE(resolved.stroke->color.value() == QColor(Qt::black));
    REQUIRE(resolved.stroke->width.has_value());
    REQUIRE(resolved.stroke->width.value() == 2.5);
}
