#pragma once

#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

namespace lumen::data {

/// Coordinate array for a dataset dimension.
///
/// Two storage modes:
/// - Regular: start + step + count (memory efficient for evenly-spaced data).
/// - Irregular: explicit vector<double> for non-uniform spacing.
///
/// Provides value lookup by index and nearest-index lookup by value.
class CoordinateArray {
public:
    /// Construct a regular coordinate array.
    CoordinateArray(double start, double step, std::size_t count);

    /// Construct an irregular coordinate array.
    explicit CoordinateArray(std::vector<double> values);

    /// Value at the given index. Throws std::out_of_range if out of bounds.
    [[nodiscard]] double valueAt(std::size_t index) const;

    /// Find the nearest index for a given value (label-based lookup).
    /// Returns std::nullopt if the array is empty.
    [[nodiscard]] std::optional<std::size_t> indexOf(double value) const;

    /// Whether the coordinates are regularly spaced.
    [[nodiscard]] bool isRegular() const;

    /// Number of coordinate values.
    [[nodiscard]] std::size_t size() const;

private:
    struct Regular {
        double start;
        double step;
        std::size_t count;
    };

    std::variant<Regular, std::vector<double>> storage_;
};

} // namespace lumen::data
