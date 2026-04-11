#pragma once

#include "Dataset.h"
#include "Dimension.h"
#include "Unit.h"

#include <QString>

#include <cstddef>
#include <vector>

namespace lumen::data {

/// Concrete rank-3 Dataset storing a 3D volume of doubles.
///
/// Data stored in row-major order: data[(z * dimY * dimX) + (y * dimX) + x].
/// Three named dimensions. Memory-manager-aware (volumes are often >100 MB).
class Volume3D : public Dataset {
    Q_OBJECT

public:
    /// Construct a 3D volume.
    /// \param name     display name
    /// \param unit     physical unit of values
    /// \param dimX     first dimension
    /// \param dimY     second dimension
    /// \param dimZ     third dimension
    /// \param data     data in row-major order, must have dimX * dimY * dimZ elements
    Volume3D(QString name, Unit unit, Dimension dimX, Dimension dimY,
             Dimension dimZ, std::vector<double> data,
             QObject* parent = nullptr);

    // Dataset interface
    [[nodiscard]] QString name() const override;
    [[nodiscard]] std::vector<Dimension> dimensions() const override;
    [[nodiscard]] Unit valueUnit() const override;
    [[nodiscard]] StorageMode storageMode() const override;
    [[nodiscard]] std::size_t rank() const override;
    [[nodiscard]] std::vector<std::size_t> shape() const override;
    [[nodiscard]] std::size_t sizeBytes() const override;
    [[nodiscard]] double valueAt(const std::vector<std::size_t>& index) const override;

    /// Dimension sizes.
    [[nodiscard]] std::size_t sizeX() const { return dimX_.length; }
    [[nodiscard]] std::size_t sizeY() const { return dimY_.length; }
    [[nodiscard]] std::size_t sizeZ() const { return dimZ_.length; }

    /// Direct access to the underlying flat data array.
    [[nodiscard]] const std::vector<double>& data() const { return data_; }

private:
    QString name_;
    Unit unit_;
    Dimension dimX_;
    Dimension dimY_;
    Dimension dimZ_;
    std::vector<double> data_;
};

} // namespace lumen::data
