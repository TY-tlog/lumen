#pragma once

#include "DatasetLoader.h"

namespace lumen::data::io {

/// NumPy .npy file loader (custom binary parser).
///
/// Reads the .npy header (magic, version, shape, dtype) and raw data.
/// Dispatches by rank:
/// - 1D -> TabularBundle (single column)
/// - 2D -> Grid2D
class NumpyLoader : public DatasetLoader {
public:
    [[nodiscard]] QStringList supportedExtensions() const override;
    [[nodiscard]] std::shared_ptr<TabularBundle> loadTabular(const QString& path) override;
    [[nodiscard]] std::shared_ptr<Dataset> loadDataset(const QString& path) override;
};

} // namespace lumen::data::io
