#pragma once

#include "DatasetLoader.h"

#ifdef LUMEN_HAS_TIFF

namespace lumen::data::io {

/// TIFF image/stack loader using libtiff.
///
/// - Single-page TIFF -> Grid2D
/// - Multi-page TIFF -> Volume3D (each page = one Z slice)
class TiffStackLoader : public DatasetLoader {
public:
    [[nodiscard]] QStringList supportedExtensions() const override;
    [[nodiscard]] std::shared_ptr<TabularBundle> loadTabular(const QString& path) override;
    [[nodiscard]] std::shared_ptr<Dataset> loadDataset(const QString& path) override;
};

} // namespace lumen::data::io

#endif // LUMEN_HAS_TIFF
