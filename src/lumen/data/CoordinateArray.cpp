#include "CoordinateArray.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace lumen::data {

CoordinateArray::CoordinateArray(double start, double step, std::size_t count)
    : storage_(Regular{start, step, count})
{
}

CoordinateArray::CoordinateArray(std::vector<double> values)
    : storage_(std::move(values))
{
}

double CoordinateArray::valueAt(std::size_t index) const
{
    if (index >= size()) {
        throw std::out_of_range("CoordinateArray::valueAt: index "
                                + std::to_string(index) + " out of range (size="
                                + std::to_string(size()) + ")");
    }
    if (const auto* reg = std::get_if<Regular>(&storage_)) {
        return reg->start + static_cast<double>(index) * reg->step;
    }
    return std::get<std::vector<double>>(storage_)[index];
}

std::optional<std::size_t> CoordinateArray::indexOf(double value) const
{
    std::size_t n = size();
    if (n == 0) {
        return std::nullopt;
    }

    if (const auto* reg = std::get_if<Regular>(&storage_)) {
        if (reg->step == 0.0) {
            return 0;
        }
        double rawIndex = (value - reg->start) / reg->step;
        auto idx = static_cast<std::size_t>(std::round(rawIndex));
        if (idx >= n) {
            idx = n - 1;
        }
        return idx;
    }

    // Irregular: linear search for nearest
    const auto& vals = std::get<std::vector<double>>(storage_);
    std::size_t best = 0;
    double bestDist = std::abs(vals[0] - value);
    for (std::size_t i = 1; i < n; ++i) {
        double dist = std::abs(vals[i] - value);
        if (dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

bool CoordinateArray::isRegular() const
{
    return std::holds_alternative<Regular>(storage_);
}

std::size_t CoordinateArray::size() const
{
    if (const auto* reg = std::get_if<Regular>(&storage_)) {
        return reg->count;
    }
    return std::get<std::vector<double>>(storage_).size();
}

} // namespace lumen::data
