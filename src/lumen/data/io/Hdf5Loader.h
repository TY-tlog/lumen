#pragma once

#include "DatasetLoader.h"

#ifdef LUMEN_HAS_HDF5

namespace lumen::data::io {

/// HDF5 file loader using the HDF5 C API.
///
/// Reads the first dataset in the file and dispatches by rank:
/// - 1D -> TabularBundle (single column)
/// - 2D -> Grid2D
/// - 3D -> Volume3D
class Hdf5Loader : public DatasetLoader {
public:
    [[nodiscard]] QStringList supportedExtensions() const override;
    [[nodiscard]] std::shared_ptr<TabularBundle> loadTabular(const QString& path) override;
    [[nodiscard]] std::shared_ptr<Dataset> loadDataset(const QString& path) override;
};

} // namespace lumen::data::io

#endif // LUMEN_HAS_HDF5
