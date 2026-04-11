#include "JsonLoader.h"

#include <data/Rank1Dataset.h>
#include <data/Unit.h>

#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace lumen::data::io {

QStringList JsonLoader::supportedExtensions() const
{
    return {QStringLiteral("json")};
}

std::shared_ptr<TabularBundle> JsonLoader::loadTabular(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("JsonLoader: cannot open file: " + path.toStdString());
    }

    QByteArray bytes = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);
    if (doc.isNull()) {
        throw std::runtime_error("JsonLoader: parse error: " + error.errorString().toStdString());
    }

    if (!doc.isArray()) {
        throw std::runtime_error("JsonLoader: expected top-level JSON array");
    }

    QJsonArray arr = doc.array();
    if (arr.isEmpty()) {
        return std::make_shared<TabularBundle>();
    }

    // First pass: collect all column names from all objects
    QStringList columnNames;
    for (const QJsonValue& val : arr) {
        if (!val.isObject()) {
            continue;
        }
        QJsonObject obj = val.toObject();
        for (const QString& key : obj.keys()) {
            if (!columnNames.contains(key)) {
                columnNames.append(key);
            }
        }
    }

    if (columnNames.isEmpty()) {
        return std::make_shared<TabularBundle>();
    }

    // Determine column types from first non-null value
    enum class ColType { Double, String };
    std::vector<ColType> colTypes(static_cast<std::size_t>(columnNames.size()), ColType::Double);

    for (const QJsonValue& val : arr) {
        if (!val.isObject()) {
            continue;
        }
        QJsonObject obj = val.toObject();
        for (int c = 0; c < columnNames.size(); ++c) {
            QJsonValue v = obj.value(columnNames[c]);
            if (v.isString()) {
                colTypes[static_cast<std::size_t>(c)] = ColType::String;
            }
        }
    }

    // Collect data
    std::size_t numCols = static_cast<std::size_t>(columnNames.size());
    std::vector<std::vector<double>> dblCols(numCols);
    std::vector<std::vector<QString>> strCols(numCols);

    for (const QJsonValue& val : arr) {
        if (!val.isObject()) {
            continue;
        }
        QJsonObject obj = val.toObject();
        for (std::size_t c = 0; c < numCols; ++c) {
            QJsonValue v = obj.value(columnNames[static_cast<int>(c)]);
            if (colTypes[c] == ColType::String) {
                strCols[c].push_back(v.isString() ? v.toString() : QString());
            } else {
                if (v.isDouble()) {
                    dblCols[c].push_back(v.toDouble());
                } else if (v.isNull() || v.isUndefined()) {
                    dblCols[c].push_back(std::numeric_limits<double>::quiet_NaN());
                } else {
                    dblCols[c].push_back(std::numeric_limits<double>::quiet_NaN());
                }
            }
        }
    }

    // Build TabularBundle
    auto bundle = std::make_shared<TabularBundle>();
    Unit unit = Unit::dimensionless();

    for (std::size_t c = 0; c < numCols; ++c) {
        std::shared_ptr<Rank1Dataset> ds;
        if (colTypes[c] == ColType::String) {
            ds = std::make_shared<Rank1Dataset>(columnNames[static_cast<int>(c)], unit,
                                                std::move(strCols[c]));
        } else {
            ds = std::make_shared<Rank1Dataset>(columnNames[static_cast<int>(c)], unit,
                                                std::move(dblCols[c]));
        }
        bundle->addColumn(std::move(ds));
    }

    return bundle;
}

} // namespace lumen::data::io
