#pragma once

#include <vector>

namespace lumen::plot {

/// Result of a nice-number tick computation.
struct TickResult {
    std::vector<double> values;
    int decimalPlaces{0};
    double spacing{0.0};
};

/// Computes "nice" tick values for an axis given a data range.
///
/// Uses the 1-2-5 pattern to find human-readable tick spacing.
/// Handles: zero range, fractional ranges, negative ranges, large ranges.
class NiceNumbers {
public:
    /// Compute nice tick values for the range [min, max].
    /// @param min  Lower bound of data range.
    /// @param max  Upper bound of data range.
    /// @param targetTickCount  Desired number of ticks (approximate).
    /// @return TickResult with tick values, decimal places for formatting, and spacing.
    [[nodiscard]] static TickResult compute(double min, double max,
                                            int targetTickCount = 7);

private:
    /// Round a number to a "nice" value (1, 2, 5 x 10^n).
    [[nodiscard]] static double niceNum(double value, bool roundDown);
};

} // namespace lumen::plot
