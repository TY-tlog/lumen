#pragma once

#include "Dimension.h"
#include "Unit.h"

#include <QObject>

#include <cstddef>
#include <vector>

namespace lumen::data {

/// Abstract base class for n-dimensional scientific datasets.
///
/// Each Dataset has a rank, shape, named dimensions with coordinate
/// arrays and units, a value unit, and a storage mode. Inherits QObject
/// for reactive signals (ADR-035).
class Dataset : public QObject {
    Q_OBJECT

public:
    enum class StorageMode { InMemory, Chunked };

    ~Dataset() override = default;

    /// Human-readable name of the dataset.
    [[nodiscard]] virtual QString name() const = 0;

    /// Ordered list of dimensions (one per rank).
    [[nodiscard]] virtual std::vector<Dimension> dimensions() const = 0;

    /// Physical unit of the data values.
    [[nodiscard]] virtual Unit valueUnit() const = 0;

    /// Storage mode (in-memory or chunked).
    [[nodiscard]] virtual StorageMode storageMode() const = 0;

    /// Number of dimensions.
    [[nodiscard]] virtual std::size_t rank() const = 0;

    /// Size along each dimension.
    [[nodiscard]] virtual std::vector<std::size_t> shape() const = 0;

    /// Total size in bytes of the stored data.
    [[nodiscard]] virtual std::size_t sizeBytes() const = 0;

    /// Access a single value by multi-dimensional index.
    [[nodiscard]] virtual double valueAt(const std::vector<std::size_t>& index) const = 0;

signals:
    /// Emitted when data values change.
    void changed();

    /// Emitted when coordinate arrays change.
    void coordinatesChanged();

protected:
    explicit Dataset(QObject* parent = nullptr)
        : QObject(parent)
    {
    }
};

} // namespace lumen::data
