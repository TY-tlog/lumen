#include "json_io.h"

#include <QJsonArray>

namespace lumen::style {

namespace {

Optional<QColor> readColor(const QJsonObject& obj, const QString& key)
{
    if (!obj.contains(key)) return std::nullopt;
    return QColor(obj[key].toString());
}

Optional<double> readDouble(const QJsonObject& obj, const QString& key)
{
    if (!obj.contains(key)) return std::nullopt;
    return obj[key].toDouble();
}

Optional<int> readInt(const QJsonObject& obj, const QString& key)
{
    if (!obj.contains(key)) return std::nullopt;
    return obj[key].toInt();
}

Optional<QString> readString(const QJsonObject& obj, const QString& key)
{
    if (!obj.contains(key)) return std::nullopt;
    return obj[key].toString();
}

Optional<bool> readBool(const QJsonObject& obj, const QString& key)
{
    if (!obj.contains(key)) return std::nullopt;
    return obj[key].toBool();
}

StrokeStyle readStroke(const QJsonObject& obj)
{
    StrokeStyle s;
    s.color = readColor(obj, QStringLiteral("color"));
    s.width = readDouble(obj, QStringLiteral("width"));
    return s;
}

FillStyle readFill(const QJsonObject& obj)
{
    FillStyle f;
    f.color = readColor(obj, QStringLiteral("color"));
    f.alpha = readDouble(obj, QStringLiteral("alpha"));
    return f;
}

TextStyle readText(const QJsonObject& obj)
{
    TextStyle t;
    t.family = readString(obj, QStringLiteral("family"));
    t.size = readDouble(obj, QStringLiteral("size"));
    t.color = readColor(obj, QStringLiteral("color"));
    return t;
}

MarkerStyle readMarker(const QJsonObject& obj)
{
    MarkerStyle m;
    m.size = readDouble(obj, QStringLiteral("size"));
    m.fillColor = readColor(obj, QStringLiteral("fillColor"));
    m.strokeColor = readColor(obj, QStringLiteral("strokeColor"));
    return m;
}

GridStyle readGrid(const QJsonObject& obj)
{
    GridStyle g;
    g.visible = readBool(obj, QStringLiteral("visible"));
    g.majorColor = readColor(obj, QStringLiteral("majorColor"));
    g.minorColor = readColor(obj, QStringLiteral("minorColor"));
    g.majorWidth = readDouble(obj, QStringLiteral("majorWidth"));
    return g;
}

void writeColor(QJsonObject& obj, const QString& key, const Optional<QColor>& val)
{
    if (val.has_value())
        obj[key] = val->name(QColor::HexArgb);
}

void writeDouble(QJsonObject& obj, const QString& key, const Optional<double>& val)
{
    if (val.has_value())
        obj[key] = *val;
}

void writeString(QJsonObject& obj, const QString& key, const Optional<QString>& val)
{
    if (val.has_value())
        obj[key] = *val;
}

void writeInt(QJsonObject& obj, const QString& key, const Optional<int>& val)
{
    if (val.has_value())
        obj[key] = *val;
}

void writeBool(QJsonObject& obj, const QString& key, const Optional<bool>& val)
{
    if (val.has_value())
        obj[key] = *val;
}

}  // namespace

Style loadStyleFromJson(const QJsonObject& obj)
{
    Style s;

    if (obj.contains(QStringLiteral("stroke")))
        s.stroke = readStroke(obj[QStringLiteral("stroke")].toObject());
    if (obj.contains(QStringLiteral("fill")))
        s.fill = readFill(obj[QStringLiteral("fill")].toObject());
    if (obj.contains(QStringLiteral("text")))
        s.text = readText(obj[QStringLiteral("text")].toObject());
    if (obj.contains(QStringLiteral("marker")))
        s.marker = readMarker(obj[QStringLiteral("marker")].toObject());
    if (obj.contains(QStringLiteral("grid")))
        s.grid = readGrid(obj[QStringLiteral("grid")].toObject());

    s.backgroundColor = readColor(obj, QStringLiteral("backgroundColor"));
    s.foregroundColor = readColor(obj, QStringLiteral("foregroundColor"));
    s.lineWidth = readDouble(obj, QStringLiteral("lineWidth"));
    s.markerSize = readDouble(obj, QStringLiteral("markerSize"));
    s.colormapName = readString(obj, QStringLiteral("colormapName"));
    s.contourLevels = readInt(obj, QStringLiteral("contourLevels"));
    s.barWidth = readDouble(obj, QStringLiteral("barWidth"));

    return s;
}

QJsonObject saveStyleToJson(const Style& style, const QString& name,
                            const QString& extends_)
{
    QJsonObject obj;
    obj[QStringLiteral("lumen_style_version")] = QStringLiteral("1.0");

    if (!name.isEmpty())
        obj[QStringLiteral("name")] = name;
    if (!extends_.isEmpty())
        obj[QStringLiteral("extends")] = extends_;

    // Stroke
    if (style.stroke.has_value()) {
        QJsonObject s;
        writeColor(s, QStringLiteral("color"), style.stroke->color);
        writeDouble(s, QStringLiteral("width"), style.stroke->width);
        if (!s.isEmpty())
            obj[QStringLiteral("stroke")] = s;
    }

    // Fill
    if (style.fill.has_value()) {
        QJsonObject f;
        writeColor(f, QStringLiteral("color"), style.fill->color);
        writeDouble(f, QStringLiteral("alpha"), style.fill->alpha);
        if (!f.isEmpty())
            obj[QStringLiteral("fill")] = f;
    }

    // Text
    if (style.text.has_value()) {
        QJsonObject t;
        writeString(t, QStringLiteral("family"), style.text->family);
        writeDouble(t, QStringLiteral("size"), style.text->size);
        writeColor(t, QStringLiteral("color"), style.text->color);
        if (!t.isEmpty())
            obj[QStringLiteral("text")] = t;
    }

    // Marker
    if (style.marker.has_value()) {
        QJsonObject m;
        writeDouble(m, QStringLiteral("size"), style.marker->size);
        writeColor(m, QStringLiteral("fillColor"), style.marker->fillColor);
        writeColor(m, QStringLiteral("strokeColor"), style.marker->strokeColor);
        if (!m.isEmpty())
            obj[QStringLiteral("marker")] = m;
    }

    // Grid
    if (style.grid.has_value()) {
        QJsonObject g;
        writeBool(g, QStringLiteral("visible"), style.grid->visible);
        writeColor(g, QStringLiteral("majorColor"), style.grid->majorColor);
        writeColor(g, QStringLiteral("minorColor"), style.grid->minorColor);
        writeDouble(g, QStringLiteral("majorWidth"), style.grid->majorWidth);
        if (!g.isEmpty())
            obj[QStringLiteral("grid")] = g;
    }

    // Top-level
    writeColor(obj, QStringLiteral("backgroundColor"), style.backgroundColor);
    writeColor(obj, QStringLiteral("foregroundColor"), style.foregroundColor);
    writeDouble(obj, QStringLiteral("lineWidth"), style.lineWidth);
    writeDouble(obj, QStringLiteral("markerSize"), style.markerSize);
    writeString(obj, QStringLiteral("colormapName"), style.colormapName);
    writeInt(obj, QStringLiteral("contourLevels"), style.contourLevels);
    writeDouble(obj, QStringLiteral("barWidth"), style.barWidth);

    return obj;
}

QString validateStyleJson(const QJsonObject& obj)
{
    // Check required field.
    if (!obj.contains(QStringLiteral("lumen_style_version")))
        return QStringLiteral("Missing required field: lumen_style_version");

    QString version = obj[QStringLiteral("lumen_style_version")].toString();
    if (version.isEmpty())
        return QStringLiteral("lumen_style_version must be a non-empty string");

    // Check version is 1.x.
    if (!version.startsWith(QStringLiteral("1.")))
        return QStringLiteral("Unsupported style version: %1 (expected 1.x)").arg(version);

    return {};
}

}  // namespace lumen::style
