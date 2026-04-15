#pragma once

#include "types.h"

#include <QJsonObject>
#include <QString>

namespace lumen::style {

/// Load a Style from a JSON object (token references resolved).
/// Returns empty Style on parse error.
Style loadStyleFromJson(const QJsonObject& obj);

/// Save a Style to a JSON object.
QJsonObject saveStyleToJson(const Style& style, const QString& name = {},
                            const QString& extends_ = {});

/// Validate a JSON object against the style schema.
/// Returns empty string on success, error message on failure.
QString validateStyleJson(const QJsonObject& obj);

}  // namespace lumen::style
