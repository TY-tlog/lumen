#include "DashboardSerializer.h"
#include "Dashboard.h"
#include "PanelConfig.h"

#include <style/json_io.h>

#include <QJsonArray>

namespace lumen::dashboard {

QJsonObject DashboardSerializer::toJson(const Dashboard& dashboard)
{
    QJsonObject obj;
    obj[QLatin1String("gridRows")] = dashboard.gridRows();
    obj[QLatin1String("gridCols")] = dashboard.gridCols();

    if (dashboard.dashboardStyle() != style::Style{}) {
        obj[QLatin1String("style")] = style::saveStyleToJson(dashboard.dashboardStyle());
    }

    QJsonArray panelsArr;
    for (int i = 0; i < dashboard.panelCount(); ++i) {
        const auto& cfg = dashboard.panelConfigAt(i);
        QJsonObject p;
        p[QLatin1String("row")] = cfg.row;
        p[QLatin1String("col")] = cfg.col;
        p[QLatin1String("rowSpan")] = cfg.rowSpan;
        p[QLatin1String("colSpan")] = cfg.colSpan;
        if (!cfg.title.isEmpty())
            p[QLatin1String("title")] = cfg.title;
        p[QLatin1String("linkGroup")] = cfg.linkGroup;

        const auto& ps = dashboard.panelStyle(i);
        if (ps != style::Style{}) {
            p[QLatin1String("panelStyle")] = style::saveStyleToJson(ps);
        }

        panelsArr.append(p);
    }
    obj[QLatin1String("panels")] = panelsArr;

    return obj;
}

void DashboardSerializer::fromJson(const QJsonObject& obj, Dashboard& dashboard)
{
    int rows = obj[QLatin1String("gridRows")].toInt(1);
    int cols = obj[QLatin1String("gridCols")].toInt(1);
    dashboard.setGridSize(rows, cols);

    if (obj.contains(QLatin1String("style"))) {
        dashboard.setDashboardStyle(
            style::loadStyleFromJson(obj[QLatin1String("style")].toObject()));
    }

    const QJsonArray panelsArr = obj[QLatin1String("panels")].toArray();
    for (const auto& val : panelsArr) {
        const QJsonObject p = val.toObject();
        PanelConfig cfg;
        cfg.row = p[QLatin1String("row")].toInt(0);
        cfg.col = p[QLatin1String("col")].toInt(0);
        cfg.rowSpan = p[QLatin1String("rowSpan")].toInt(1);
        cfg.colSpan = p[QLatin1String("colSpan")].toInt(1);
        cfg.title = p[QLatin1String("title")].toString();
        cfg.linkGroup = p[QLatin1String("linkGroup")].toString(
            QStringLiteral("default"));

        int idx = dashboard.addPanel(cfg);

        if (p.contains(QLatin1String("panelStyle"))) {
            dashboard.setPanelStyle(idx,
                style::loadStyleFromJson(p[QLatin1String("panelStyle")].toObject()));
        }
    }
}

QJsonObject DashboardSerializer::migrateV1toV2(const QJsonObject& v1Root)
{
    QJsonObject v2 = v1Root;
    v2[QLatin1String("version")] = 2;

    QJsonObject dashboard;
    dashboard[QLatin1String("gridRows")] = 1;
    dashboard[QLatin1String("gridCols")] = 1;

    QJsonObject panel;
    panel[QLatin1String("row")] = 0;
    panel[QLatin1String("col")] = 0;
    panel[QLatin1String("rowSpan")] = 1;
    panel[QLatin1String("colSpan")] = 1;
    panel[QLatin1String("linkGroup")] = QStringLiteral("default");

    if (v1Root.contains(QLatin1String("plot"))) {
        panel[QLatin1String("plot")] = v1Root[QLatin1String("plot")];
    }

    QJsonArray panels;
    panels.append(panel);
    dashboard[QLatin1String("panels")] = panels;

    v2[QLatin1String("dashboard")] = dashboard;
    return v2;
}

}  // namespace lumen::dashboard
