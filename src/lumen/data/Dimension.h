#pragma once

#include "CoordinateArray.h"
#include "Unit.h"

#include <QString>

#include <cstddef>

namespace lumen::data {

/// A named dimension of a Dataset, with unit, length, and coordinate values.
struct Dimension {
    QString name;
    Unit unit;
    std::size_t length;
    CoordinateArray coordinates;
};

} // namespace lumen::data
