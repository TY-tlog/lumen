#include "Rank1Dataset.h"

#include "CoordinateArray.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace lumen::data {

Rank1Dataset::Rank1Dataset(QString name, Unit unit, std::vector<double> data,
                           QObject* parent)
    : Dataset(parent)
    , name_(std::move(name))
    , unit_(std::move(unit))
    , storage_(std::move(data))
{
}

Rank1Dataset::Rank1Dataset(QString name, Unit unit, std::vector<int64_t> data,
                           QObject* parent)
    : Dataset(parent)
    , name_(std::move(name))
    , unit_(std::move(unit))
    , storage_(std::move(data))
{
}

Rank1Dataset::Rank1Dataset(QString name, Unit unit, std::vector<QString> data,
                           QObject* parent)
    : Dataset(parent)
    , name_(std::move(name))
    , unit_(std::move(unit))
    , storage_(std::move(data))
{
}

QString Rank1Dataset::name() const
{
    return name_;
}

std::vector<Dimension> Rank1Dataset::dimensions() const
{
    std::size_t n = rowCount();
    return {Dimension{
        QStringLiteral("row"),
        Unit::dimensionless(),
        n,
        CoordinateArray(0.0, 1.0, n),
    }};
}

Unit Rank1Dataset::valueUnit() const
{
    return unit_;
}

Dataset::StorageMode Rank1Dataset::storageMode() const
{
    return StorageMode::InMemory;
}

std::size_t Rank1Dataset::rank() const
{
    return 1;
}

std::vector<std::size_t> Rank1Dataset::shape() const
{
    return {rowCount()};
}

std::size_t Rank1Dataset::sizeBytes() const
{
    return std::visit(
        [](const auto& vec) -> std::size_t {
            using T = std::decay_t<decltype(vec)>;
            if constexpr (std::is_same_v<T, std::vector<double>>) {
                return vec.size() * sizeof(double);
            } else if constexpr (std::is_same_v<T, std::vector<int64_t>>) {
                return vec.size() * sizeof(int64_t);
            } else {
                // QString: approximate
                std::size_t total = 0;
                for (const auto& s : vec) {
                    total += static_cast<std::size_t>(s.size()) * sizeof(QChar);
                }
                return total;
            }
        },
        storage_);
}

double Rank1Dataset::valueAt(const std::vector<std::size_t>& index) const
{
    if (index.size() != 1) {
        throw std::invalid_argument(
            "Rank1Dataset::valueAt: expected 1 index, got "
            + std::to_string(index.size()));
    }
    std::size_t i = index[0];
    if (i >= rowCount()) {
        throw std::out_of_range("Rank1Dataset::valueAt: index "
                                + std::to_string(i) + " out of range (size="
                                + std::to_string(rowCount()) + ")");
    }
    if (const auto* dv = std::get_if<std::vector<double>>(&storage_)) {
        return (*dv)[i];
    }
    if (const auto* iv = std::get_if<std::vector<int64_t>>(&storage_)) {
        return static_cast<double>((*iv)[i]);
    }
    throw std::invalid_argument("Rank1Dataset::valueAt: string data has no double representation");
}

const std::vector<double>& Rank1Dataset::doubleData() const
{
    return std::get<std::vector<double>>(storage_);
}

const std::vector<int64_t>& Rank1Dataset::int64Data() const
{
    return std::get<std::vector<int64_t>>(storage_);
}

const std::vector<QString>& Rank1Dataset::stringData() const
{
    return std::get<std::vector<QString>>(storage_);
}

std::size_t Rank1Dataset::rowCount() const
{
    return std::visit([](const auto& vec) -> std::size_t { return vec.size(); }, storage_);
}

std::unique_ptr<Dataset> Rank1Dataset::clone() const
{
    return std::visit(
        [this](const auto& vec) -> std::unique_ptr<Dataset> {
            using Vec = std::decay_t<decltype(vec)>;
            Vec clonedData = vec;
            return std::make_unique<Rank1Dataset>(name_, unit_, std::move(clonedData));
        },
        storage_);
}

} // namespace lumen::data
