#include "TabularBundle.h"

#include <stdexcept>
#include <string>

namespace lumen::data {

void TabularBundle::addColumn(std::shared_ptr<Rank1Dataset> col)
{
    if (!col) {
        throw std::invalid_argument("TabularBundle::addColumn: null column");
    }
    if (!columns_.empty()) {
        std::size_t expected = columns_[0]->rowCount();
        if (col->rowCount() != expected) {
            throw std::invalid_argument(
                "TabularBundle::addColumn: column '" + col->name().toStdString()
                + "' has " + std::to_string(col->rowCount()) + " rows, expected "
                + std::to_string(expected));
        }
    }
    columns_.push_back(std::move(col));
}

std::shared_ptr<Rank1Dataset> TabularBundle::column(int index) const
{
    if (index < 0 || static_cast<std::size_t>(index) >= columns_.size()) {
        return nullptr;
    }
    return columns_[static_cast<std::size_t>(index)];
}

std::shared_ptr<Rank1Dataset> TabularBundle::columnByName(const QString& name) const
{
    for (const auto& col : columns_) {
        if (col->name() == name) {
            return col;
        }
    }
    return nullptr;
}

int TabularBundle::columnCount() const
{
    return static_cast<int>(columns_.size());
}

std::size_t TabularBundle::rowCount() const
{
    if (columns_.empty()) {
        return 0;
    }
    return columns_[0]->rowCount();
}

QStringList TabularBundle::columnNames() const
{
    QStringList names;
    names.reserve(static_cast<int>(columns_.size()));
    for (const auto& col : columns_) {
        names.append(col->name());
    }
    return names;
}

} // namespace lumen::data
