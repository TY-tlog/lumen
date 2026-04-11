#pragma once

#include "DatasetLoader.h"

namespace lumen::data::io {

/// CSV file loader wrapping CsvReader. Produces TabularBundle.
class CsvLoader : public DatasetLoader {
public:
    [[nodiscard]] QStringList supportedExtensions() const override;
    [[nodiscard]] std::shared_ptr<TabularBundle> loadTabular(const QString& path) override;
};

} // namespace lumen::data::io
