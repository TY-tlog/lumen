#pragma once

#include "Dataset.h"
#include "Dimension.h"
#include "Unit.h"

#include <QString>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

namespace lumen::data {

/// Concrete rank-1 Dataset backed by a vector of doubles, int64s, or QStrings.
///
/// Replaces Column for the new Dataset model. Each Rank1Dataset has a name,
/// a value unit, and one dimension (the row/index dimension).
class Rank1Dataset : public Dataset {
    Q_OBJECT

public:
    /// Construct a double-valued rank-1 dataset.
    Rank1Dataset(QString name, Unit unit, std::vector<double> data,
                 QObject* parent = nullptr);

    /// Construct an int64-valued rank-1 dataset.
    Rank1Dataset(QString name, Unit unit, std::vector<int64_t> data,
                 QObject* parent = nullptr);

    /// Construct a string-valued rank-1 dataset.
    Rank1Dataset(QString name, Unit unit, std::vector<QString> data,
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

    /// Direct access to double data. Throws std::bad_variant_access if wrong type.
    [[nodiscard]] const std::vector<double>& doubleData() const;

    /// Direct access to int64 data. Throws std::bad_variant_access if wrong type.
    [[nodiscard]] const std::vector<int64_t>& int64Data() const;

    /// Direct access to string data. Throws std::bad_variant_access if wrong type.
    [[nodiscard]] const std::vector<QString>& stringData() const;

    /// Number of rows (convenience).
    [[nodiscard]] std::size_t rowCount() const;

private:
    QString name_;
    Unit unit_;
    std::variant<std::vector<double>, std::vector<int64_t>, std::vector<QString>> storage_;
};

} // namespace lumen::data
