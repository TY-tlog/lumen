#include "Volume3D.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace lumen::data {

Volume3D::Volume3D(QString name, Unit unit, Dimension dimX, Dimension dimY,
                   Dimension dimZ, std::vector<double> data, QObject* parent)
    : Dataset(parent)
    , name_(std::move(name))
    , unit_(std::move(unit))
    , dimX_(std::move(dimX))
    , dimY_(std::move(dimY))
    , dimZ_(std::move(dimZ))
    , data_(std::move(data))
{
    std::size_t expected = dimX_.length * dimY_.length * dimZ_.length;
    if (data_.size() != expected) {
        throw std::invalid_argument(
            "Volume3D: data size " + std::to_string(data_.size())
            + " does not match dimensions " + std::to_string(dimX_.length)
            + " x " + std::to_string(dimY_.length) + " x "
            + std::to_string(dimZ_.length) + " = " + std::to_string(expected));
    }
}

QString Volume3D::name() const
{
    return name_;
}

std::vector<Dimension> Volume3D::dimensions() const
{
    return {dimX_, dimY_, dimZ_};
}

Unit Volume3D::valueUnit() const
{
    return unit_;
}

Dataset::StorageMode Volume3D::storageMode() const
{
    return StorageMode::InMemory;
}

std::size_t Volume3D::rank() const
{
    return 3;
}

std::vector<std::size_t> Volume3D::shape() const
{
    return {dimX_.length, dimY_.length, dimZ_.length};
}

std::size_t Volume3D::sizeBytes() const
{
    return data_.size() * sizeof(double);
}

double Volume3D::valueAt(const std::vector<std::size_t>& index) const
{
    if (index.size() != 3) {
        throw std::invalid_argument(
            "Volume3D::valueAt: expected 3 indices, got "
            + std::to_string(index.size()));
    }
    std::size_t x = index[0];
    std::size_t y = index[1];
    std::size_t z = index[2];
    if (x >= dimX_.length || y >= dimY_.length || z >= dimZ_.length) {
        throw std::out_of_range(
            "Volume3D::valueAt: index (" + std::to_string(x) + ", "
            + std::to_string(y) + ", " + std::to_string(z) + ") out of range ("
            + std::to_string(dimX_.length) + ", " + std::to_string(dimY_.length)
            + ", " + std::to_string(dimZ_.length) + ")");
    }
    return data_[(z * dimY_.length * dimX_.length) + (y * dimX_.length) + x];
}

} // namespace lumen::data
