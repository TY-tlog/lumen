#pragma once

#include "Dataset.h"
#include "Dimension.h"
#include "Unit.h"

#include <QString>

#include <cstddef>
#include <memory>
#include <vector>

namespace lumen::data {

/// Concrete rank-2 Dataset storing a 2D grid of doubles.
///
/// Data is stored in row-major order: data[row * cols + col].
/// Two named dimensions (typically x and y) with coordinate arrays.
class Grid2D : public Dataset {
    Q_OBJECT

public:
    /// Construct a 2D grid.
    /// \param name     display name
    /// \param unit     physical unit of values
    /// \param dimX     first dimension (columns)
    /// \param dimY     second dimension (rows)
    /// \param data     row-major data, must have dimX.length * dimY.length elements
    Grid2D(QString name, Unit unit, Dimension dimX, Dimension dimY,
           std::vector<double> data, QObject* parent = nullptr);

    // Dataset interface
    [[nodiscard]] QString name() const override;
    [[nodiscard]] std::vector<Dimension> dimensions() const override;
    [[nodiscard]] Unit valueUnit() const override;
    [[nodiscard]] StorageMode storageMode() const override;
    [[nodiscard]] std::size_t rank() const override;
    [[nodiscard]] std::vector<std::size_t> shape() const override;
    [[nodiscard]] std::size_t sizeBytes() const override;
    [[nodiscard]] double valueAt(const std::vector<std::size_t>& index) const override;
    [[nodiscard]] std::unique_ptr<Dataset> clone() const override;

    /// Number of rows (dimY length).
    [[nodiscard]] std::size_t rows() const { return dimY_.length; }

    /// Number of columns (dimX length).
    [[nodiscard]] std::size_t cols() const { return dimX_.length; }

    /// Direct access to the underlying flat data array.
    [[nodiscard]] const std::vector<double>& data() const { return data_; }

private:
    QString name_;
    Unit unit_;
    Dimension dimX_;
    Dimension dimY_;
    std::vector<double> data_;
};

} // namespace lumen::data
