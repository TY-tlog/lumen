#pragma once

#include "DatasetLoader.h"

#include <QStringList>

#include <memory>
#include <vector>

namespace lumen::data::io {

/// Singleton registry mapping file extensions to DatasetLoader instances.
///
/// Built-in loaders are registered at app startup. Phase 16 will add
/// dynamic loading of shared libraries that register additional loaders.
class LoaderRegistry {
public:
    /// Access the singleton instance.
    static LoaderRegistry& instance();

    /// Register a loader. The registry takes ownership.
    void registerLoader(std::unique_ptr<DatasetLoader> loader);

    /// Find the loader for a given file path (by extension).
    /// Returns nullptr if no loader handles the extension.
    [[nodiscard]] DatasetLoader* loaderForPath(const QString& path) const;

    /// All extensions supported across all registered loaders.
    [[nodiscard]] QStringList allSupportedExtensions() const;

    /// Build a file dialog filter string, e.g. "CSV (*.csv);;HDF5 (*.h5 *.hdf5);;..."
    [[nodiscard]] QString fileFilter() const;

    /// Number of registered loaders.
    [[nodiscard]] int loaderCount() const;

    /// Clear all registered loaders (mainly for testing).
    void clear();

private:
    LoaderRegistry() = default;

    std::vector<std::unique_ptr<DatasetLoader>> loaders_;
};

} // namespace lumen::data::io
