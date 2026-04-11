#pragma once

#include "Rank1Dataset.h"

#include <QStringList>

#include <cstddef>
#include <memory>
#include <vector>

namespace lumen::data {

/// Groups multiple Rank1Datasets sharing a row dimension.
///
/// NOT a Dataset subclass. Provides column-oriented access (by index or name)
/// for tabular data. This is the v2 replacement for DataFrame.
class TabularBundle {
public:
    TabularBundle() = default;

    /// Add a column (Rank1Dataset) to the bundle.
    /// All columns must have the same row count as existing columns.
    /// Throws std::invalid_argument if row count mismatches.
    void addColumn(std::shared_ptr<Rank1Dataset> col);

    /// Access column by index. Returns nullptr if out of bounds.
    [[nodiscard]] std::shared_ptr<Rank1Dataset> column(int index) const;

    /// Access column by name. Returns nullptr if not found.
    [[nodiscard]] std::shared_ptr<Rank1Dataset> columnByName(const QString& name) const;

    /// Number of columns.
    [[nodiscard]] int columnCount() const;

    /// Number of rows (0 if no columns).
    [[nodiscard]] std::size_t rowCount() const;

    /// Ordered list of column names.
    [[nodiscard]] QStringList columnNames() const;

private:
    std::vector<std::shared_ptr<Rank1Dataset>> columns_;
};

} // namespace lumen::data
