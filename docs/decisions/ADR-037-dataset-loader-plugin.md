# ADR-037: DatasetLoader plugin interface with extension registry

## Status
Accepted (Phase 6)

## Context
Phase 6 adds 9 file format loaders. Phase 16 adds user-defined
plugins. The loader architecture must support both built-in formats
and future user extensions.

## Decision
DatasetLoader is an abstract interface. Each format implements it.
LoaderRegistry (singleton) maps file extensions to loader instances.
Built-in loaders are registered at app startup. Phase 16 will add
dynamic loading of shared libraries that register additional
loaders.

```cpp
class DatasetLoader {
public:
    virtual ~DatasetLoader() = default;
    virtual QStringList supportedExtensions() const = 0;
    virtual std::unique_ptr<Dataset> loadDataset(const QString&) = 0;
    virtual bool canLoad(const QString& path) const;
};

class LoaderRegistry {
public:
    static LoaderRegistry& instance();
    void registerLoader(std::unique_ptr<DatasetLoader> loader);
    DatasetLoader* loaderForPath(const QString& path) const;
    QStringList allSupportedExtensions() const;
};
```

File → Open uses LoaderRegistry to find the right loader by
extension. If no loader matches, falls back to CSV.

## Consequences
- + New formats added by implementing one interface
- + Phase 16 user plugins register through the same mechanism
- + File → Open filter auto-updates from registry
- + Each loader is independently testable
- - Registry is a singleton (acceptable; there's exactly one set of
  loaders per app instance)
- - Dynamic plugin loading (Phase 16) requires platform-specific
  shared library loading. Phase 6 uses static registration only.

## Alternatives considered
- **Hardcoded format switch**: `if (ext == "csv") loadCsv(); else
  if (ext == "hdf5") loadHdf5(); ...`. Rejected: every new format
  requires modifying the switch. Phase 16 user plugins would be
  impossible.
- **CMake-time registration**: formats registered via CMake macros
  that generate a static array. Rejected: no runtime extensibility
  for Phase 16.
