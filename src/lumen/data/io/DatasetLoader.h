#pragma once

#include <data/Dataset.h>
#include <data/TabularBundle.h>

#include <QStringList>

#include <memory>

namespace lumen::data::io {

/// Abstract interface for dataset file loaders.
///
/// Each concrete loader implements supportedExtensions() and one or both
/// of loadTabular() / loadDataset(). LoaderRegistry dispatches to the
/// correct loader based on file extension.
///
/// Plugin-ready: Phase 16 will add dynamic loading of shared libraries
/// that register additional loaders (ADR-037).
class DatasetLoader {
public:
    virtual ~DatasetLoader() = default;

    /// File extensions this loader handles (without leading dot), e.g. {"csv", "tsv"}.
    [[nodiscard]] virtual QStringList supportedExtensions() const = 0;

    /// Load a file into a TabularBundle (for tabular/1D data).
    /// Returns nullptr if the format does not produce tabular data.
    [[nodiscard]] virtual std::shared_ptr<TabularBundle> loadTabular(const QString& path) = 0;

    /// Load a file into a Dataset (Grid2D, Volume3D, or Rank1Dataset).
    /// Returns nullptr if the format does not produce a single dataset.
    [[nodiscard]] virtual std::shared_ptr<Dataset> loadDataset(const QString& path);

    /// Check if this loader can handle the given file path (by extension).
    [[nodiscard]] virtual bool canLoad(const QString& path) const;
};

} // namespace lumen::data::io
