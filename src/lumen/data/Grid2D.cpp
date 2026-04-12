#include "Grid2D.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace lumen::data {

Grid2D::Grid2D(QString name, Unit unit, Dimension dimX, Dimension dimY,
               std::vector<double> data, QObject* parent)
    : Dataset(parent)
    , name_(std::move(name))
    , unit_(std::move(unit))
    , dimX_(std::move(dimX))
    , dimY_(std::move(dimY))
    , data_(std::move(data))
{
    std::size_t expected = dimX_.length * dimY_.length;
    if (data_.size() != expected) {
        throw std::invalid_argument(
            "Grid2D: data size " + std::to_string(data_.size())
            + " does not match dimensions " + std::to_string(dimX_.length)
            + " x " + std::to_string(dimY_.length) + " = "
            + std::to_string(expected));
    }
}

QString Grid2D::name() const
{
    return name_;
}

std::vector<Dimension> Grid2D::dimensions() const
{
    return {dimX_, dimY_};
}

Unit Grid2D::valueUnit() const
{
    return unit_;
}

Dataset::StorageMode Grid2D::storageMode() const
{
    return StorageMode::InMemory;
}

std::size_t Grid2D::rank() const
{
    return 2;
}

std::vector<std::size_t> Grid2D::shape() const
{
    return {dimX_.length, dimY_.length};
}

std::size_t Grid2D::sizeBytes() const
{
    return data_.size() * sizeof(double);
}

double Grid2D::valueAt(const std::vector<std::size_t>& index) const
{
    if (index.size() != 2) {
        throw std::invalid_argument(
            "Grid2D::valueAt: expected 2 indices, got "
            + std::to_string(index.size()));
    }
    std::size_t i = index[0];
    std::size_t j = index[1];
    if (i >= dimX_.length || j >= dimY_.length) {
        throw std::out_of_range(
            "Grid2D::valueAt: index (" + std::to_string(i) + ", "
            + std::to_string(j) + ") out of range (" + std::to_string(dimX_.length)
            + ", " + std::to_string(dimY_.length) + ")");
    }
    return data_[j * dimX_.length + i];
}

std::unique_ptr<Dataset> Grid2D::clone() const
{
    Dimension clonedX = dimX_;
    Dimension clonedY = dimY_;
    std::vector<double> clonedData = data_;
    return std::make_unique<Grid2D>(
        name_, unit_, std::move(clonedX), std::move(clonedY),
        std::move(clonedData));
}

} // namespace lumen::data
