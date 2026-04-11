#pragma once

#include "DatasetLoader.h"

#ifdef LUMEN_HAS_NETCDF

namespace lumen::data::io {

/// NetCDF file loader using the netcdf C API.
///
/// Reads the first numeric variable and dispatches by rank:
/// - 1D -> TabularBundle (single column)
/// - 2D -> Grid2D
/// - 3D -> Volume3D
class NetCDFLoader : public DatasetLoader {
public:
    [[nodiscard]] QStringList supportedExtensions() const override;
    [[nodiscard]] std::shared_ptr<TabularBundle> loadTabular(const QString& path) override;
    [[nodiscard]] std::shared_ptr<Dataset> loadDataset(const QString& path) override;
};

} // namespace lumen::data::io

#endif // LUMEN_HAS_NETCDF
