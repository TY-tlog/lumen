#pragma once

#include "DatasetLoader.h"

namespace lumen::data::io {

/// JSON file loader using QJsonDocument.
///
/// Expects array-of-objects format: [{"col1": 1, "col2": 2}, ...]
/// Returns TabularBundle.
class JsonLoader : public DatasetLoader {
public:
    [[nodiscard]] QStringList supportedExtensions() const override;
    [[nodiscard]] std::shared_ptr<TabularBundle> loadTabular(const QString& path) override;
};

} // namespace lumen::data::io
