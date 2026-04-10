#include "WorkspaceFile.h"

#include "data/DataFrame.h"
#include "plot/Axis.h"
#include "plot/Legend.h"
#include "plot/LineSeries.h"
#include "plot/PlotScene.h"
#include "plot/PlotStyle.h"
#include "plot/ViewTransform.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

namespace lumen::core::io {

namespace {

// --- Enum ↔ string helpers ---

QString rangeModeToString(plot::RangeMode mode)
{
    switch (mode) {
    case plot::RangeMode::Auto:   return QStringLiteral("auto");
    case plot::RangeMode::Manual: return QStringLiteral("manual");
    }
    return QStringLiteral("auto");
}

plot::RangeMode rangeModeFromString(const QString& s)
{
    if (s == QLatin1String("manual")) return plot::RangeMode::Manual;
    return plot::RangeMode::Auto;
}

QString tickFormatToString(plot::TickFormat fmt)
{
    switch (fmt) {
    case plot::TickFormat::Auto:       return QStringLiteral("auto");
    case plot::TickFormat::Scientific: return QStringLiteral("scientific");
    case plot::TickFormat::Fixed:      return QStringLiteral("fixed");
    }
    return QStringLiteral("auto");
}

plot::TickFormat tickFormatFromString(const QString& s)
{
    if (s == QLatin1String("scientific")) return plot::TickFormat::Scientific;
    if (s == QLatin1String("fixed"))      return plot::TickFormat::Fixed;
    return plot::TickFormat::Auto;
}

QString legendPositionToString(plot::LegendPosition pos)
{
    switch (pos) {
    case plot::LegendPosition::TopLeft:      return QStringLiteral("top_left");
    case plot::LegendPosition::TopRight:     return QStringLiteral("top_right");
    case plot::LegendPosition::BottomLeft:   return QStringLiteral("bottom_left");
    case plot::LegendPosition::BottomRight:  return QStringLiteral("bottom_right");
    case plot::LegendPosition::OutsideRight: return QStringLiteral("outside_right");
    }
    return QStringLiteral("top_right");
}

plot::LegendPosition legendPositionFromString(const QString& s)
{
    if (s == QLatin1String("top_left"))      return plot::LegendPosition::TopLeft;
    if (s == QLatin1String("top_right"))     return plot::LegendPosition::TopRight;
    if (s == QLatin1String("bottom_left"))   return plot::LegendPosition::BottomLeft;
    if (s == QLatin1String("bottom_right"))  return plot::LegendPosition::BottomRight;
    if (s == QLatin1String("outside_right")) return plot::LegendPosition::OutsideRight;
    return plot::LegendPosition::TopRight;
}

QString penStyleToString(Qt::PenStyle ps)
{
    switch (ps) {
    case Qt::SolidLine:      return QStringLiteral("solid");
    case Qt::DashLine:       return QStringLiteral("dash");
    case Qt::DotLine:        return QStringLiteral("dot");
    case Qt::DashDotLine:    return QStringLiteral("dashdot");
    case Qt::DashDotDotLine: return QStringLiteral("dashdotdot");
    default:                 return QStringLiteral("solid");
    }
}

Qt::PenStyle penStyleFromString(const QString& s)
{
    if (s == QLatin1String("dash"))        return Qt::DashLine;
    if (s == QLatin1String("dot"))         return Qt::DotLine;
    if (s == QLatin1String("dashdot"))     return Qt::DashDotLine;
    if (s == QLatin1String("dashdotdot")) return Qt::DashDotDotLine;
    return Qt::SolidLine;
}

// --- Axis serialization ---

QJsonObject serializeAxis(const plot::Axis& axis)
{
    QJsonObject obj;
    obj[QLatin1String("label")]              = axis.label();
    obj[QLatin1String("rangeMode")]          = rangeModeToString(axis.rangeMode());
    obj[QLatin1String("manualMin")]          = axis.manualMin();
    obj[QLatin1String("manualMax")]          = axis.manualMax();
    obj[QLatin1String("tickCount")]          = axis.tickCount();
    obj[QLatin1String("tickFormat")]         = tickFormatToString(axis.tickFormat());
    obj[QLatin1String("tickFormatDecimals")] = axis.tickFormatDecimals();
    obj[QLatin1String("gridVisible")]        = axis.gridVisible();
    return obj;
}

void deserializeAxis(const QJsonObject& obj, plot::Axis& axis)
{
    if (obj.contains(QLatin1String("label")))
        axis.setLabel(obj[QLatin1String("label")].toString());

    if (obj.contains(QLatin1String("rangeMode")))
        axis.setRangeMode(rangeModeFromString(obj[QLatin1String("rangeMode")].toString()));

    if (obj.contains(QLatin1String("manualMin")) && obj.contains(QLatin1String("manualMax")))
        axis.setManualRange(obj[QLatin1String("manualMin")].toDouble(),
                            obj[QLatin1String("manualMax")].toDouble());

    if (obj.contains(QLatin1String("tickCount")))
        axis.setTickCount(obj[QLatin1String("tickCount")].toInt());

    if (obj.contains(QLatin1String("tickFormat")))
        axis.setTickFormat(tickFormatFromString(obj[QLatin1String("tickFormat")].toString()));

    if (obj.contains(QLatin1String("tickFormatDecimals")))
        axis.setTickFormatDecimals(obj[QLatin1String("tickFormatDecimals")].toInt());

    if (obj.contains(QLatin1String("gridVisible")))
        axis.setGridVisible(obj[QLatin1String("gridVisible")].toBool(true));
}

}  // namespace

// --- WorkspaceFile ---

WorkspaceFile WorkspaceFile::loadFromPath(const QString& path)
{
    WorkspaceFile ws;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return ws;

    const QByteArray raw = file.readAll();
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return ws;

    ws.data_ = doc.object();

    // Require version field and a plot object.
    if (!ws.data_.contains(QLatin1String("version")) ||
        !ws.data_.contains(QLatin1String("plot")))
        return ws;

    ws.valid_ = true;
    return ws;
}

void WorkspaceFile::saveToPath(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QJsonDocument doc(data_);
    file.write(doc.toJson(QJsonDocument::Indented));
}

WorkspaceFile WorkspaceFile::captureFromScene(const plot::PlotScene* scene)
{
    WorkspaceFile ws;
    if (!scene) return ws;

    QJsonObject root;
    root[QLatin1String("version")] = 1;

    QJsonObject plotObj;

    // Viewport
    const auto& vt = scene->viewTransform();
    QJsonObject viewport;
    viewport[QLatin1String("xmin")] = vt.xMin();
    viewport[QLatin1String("xmax")] = vt.xMax();
    viewport[QLatin1String("ymin")] = vt.yMin();
    viewport[QLatin1String("ymax")] = vt.yMax();
    plotObj[QLatin1String("viewport")] = viewport;

    // Title
    QJsonObject titleObj;
    titleObj[QLatin1String("text")]   = scene->title();
    titleObj[QLatin1String("fontPx")] = scene->titleFontPx();
    titleObj[QLatin1String("weight")] = static_cast<int>(scene->titleWeight());
    plotObj[QLatin1String("title")] = titleObj;

    // Axes
    plotObj[QLatin1String("xAxis")] = serializeAxis(scene->xAxis());
    plotObj[QLatin1String("yAxis")] = serializeAxis(scene->yAxis());

    // Legend
    QJsonObject legendObj;
    legendObj[QLatin1String("position")] = legendPositionToString(scene->legend().position());
    legendObj[QLatin1String("visible")]  = scene->legend().isVisible();
    plotObj[QLatin1String("legend")] = legendObj;

    // Series
    QJsonArray seriesArr;
    for (const auto& s : scene->series()) {
        QJsonObject sObj;
        sObj[QLatin1String("xColumn")]   = s.xColumn() ? s.xColumn()->name() : QString();
        sObj[QLatin1String("yColumn")]   = s.yColumn() ? s.yColumn()->name() : QString();
        sObj[QLatin1String("color")]     = s.style().color.name(QColor::HexRgb);
        sObj[QLatin1String("lineWidth")] = s.style().lineWidth;
        sObj[QLatin1String("lineStyle")] = penStyleToString(s.style().penStyle);
        sObj[QLatin1String("name")]      = s.name();
        sObj[QLatin1String("visible")]   = s.isVisible();
        seriesArr.append(sObj);
    }
    plotObj[QLatin1String("series")] = seriesArr;

    root[QLatin1String("plot")] = plotObj;
    ws.data_  = root;
    ws.valid_ = true;
    return ws;
}

void WorkspaceFile::applyToScene(plot::PlotScene* scene,
                                 const data::DataFrame* df) const
{
    if (!valid_ || !scene) return;

    const QJsonObject plotObj = data_[QLatin1String("plot")].toObject();

    // Viewport
    if (plotObj.contains(QLatin1String("viewport"))) {
        const QJsonObject vp = plotObj[QLatin1String("viewport")].toObject();
        const double xmin = vp[QLatin1String("xmin")].toDouble(0.0);
        const double xmax = vp[QLatin1String("xmax")].toDouble(1.0);
        const double ymin = vp[QLatin1String("ymin")].toDouble(0.0);
        const double ymax = vp[QLatin1String("ymax")].toDouble(1.0);
        scene->viewTransform().setBaseRange(xmin, xmax, ymin, ymax);
        scene->viewTransform().reset();
    }

    // Title
    if (plotObj.contains(QLatin1String("title"))) {
        const QJsonObject t = plotObj[QLatin1String("title")].toObject();
        if (t.contains(QLatin1String("text")))
            scene->setTitle(t[QLatin1String("text")].toString());
        if (t.contains(QLatin1String("fontPx")))
            scene->setTitleFontPx(t[QLatin1String("fontPx")].toInt(17));
        if (t.contains(QLatin1String("weight")))
            scene->setTitleWeight(
                static_cast<QFont::Weight>(t[QLatin1String("weight")].toInt(
                    static_cast<int>(QFont::DemiBold))));
    }

    // Axes
    if (plotObj.contains(QLatin1String("xAxis")))
        deserializeAxis(plotObj[QLatin1String("xAxis")].toObject(), scene->xAxis());
    if (plotObj.contains(QLatin1String("yAxis")))
        deserializeAxis(plotObj[QLatin1String("yAxis")].toObject(), scene->yAxis());

    // Legend
    if (plotObj.contains(QLatin1String("legend"))) {
        const QJsonObject lg = plotObj[QLatin1String("legend")].toObject();
        if (lg.contains(QLatin1String("position")))
            scene->legend().setPosition(
                legendPositionFromString(lg[QLatin1String("position")].toString()));
        if (lg.contains(QLatin1String("visible")))
            scene->legend().setVisible(lg[QLatin1String("visible")].toBool(true));
    }

    // Series — resolve column names against the DataFrame.
    if (plotObj.contains(QLatin1String("series")) && df) {
        scene->clearSeries();
        const QJsonArray seriesArr = plotObj[QLatin1String("series")].toArray();
        for (const auto& val : seriesArr) {
            const QJsonObject sObj = val.toObject();
            const QString xColName = sObj[QLatin1String("xColumn")].toString();
            const QString yColName = sObj[QLatin1String("yColumn")].toString();

            const data::Column* xCol = df->columnByName(xColName);
            const data::Column* yCol = df->columnByName(yColName);

            if (!xCol || !yCol) continue;  // skip unresolvable series

            plot::PlotStyle style;
            style.color    = QColor(sObj[QLatin1String("color")].toString(QStringLiteral("#0a84ff")));
            style.lineWidth = sObj[QLatin1String("lineWidth")].toDouble(1.5);
            style.penStyle  = penStyleFromString(sObj[QLatin1String("lineStyle")].toString());

            const QString name    = sObj[QLatin1String("name")].toString();
            const bool    visible = sObj[QLatin1String("visible")].toBool(true);

            plot::LineSeries series(xCol, yCol, style, name);
            series.setVisible(visible);
            scene->addSeries(std::move(series));
        }
    }
}

bool WorkspaceFile::isValid() const { return valid_; }

int WorkspaceFile::version() const
{
    if (!valid_) return 0;
    return data_[QLatin1String("version")].toInt(0);
}

}  // namespace lumen::core::io
