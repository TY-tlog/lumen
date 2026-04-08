#include "plot/NiceNumbers.h"

#include <algorithm>
#include <cmath>

namespace lumen::plot {

double NiceNumbers::niceNum(double value, bool roundDown)
{
    if (value <= 0.0) {
        return 0.0;
    }

    const double exponent = std::floor(std::log10(value));
    const double fraction = value / std::pow(10.0, exponent);

    double niceFraction = 0.0;
    if (roundDown) {
        if (fraction < 1.5) {
            niceFraction = 1.0;
        } else if (fraction < 3.0) {
            niceFraction = 2.0;
        } else if (fraction < 7.0) {
            niceFraction = 5.0;
        } else {
            niceFraction = 10.0;
        }
    } else {
        if (fraction <= 1.0) {
            niceFraction = 1.0;
        } else if (fraction <= 2.0) {
            niceFraction = 2.0;
        } else if (fraction <= 5.0) {
            niceFraction = 5.0;
        } else {
            niceFraction = 10.0;
        }
    }

    return niceFraction * std::pow(10.0, exponent);
}

TickResult NiceNumbers::compute(double min, double max, int targetTickCount)
{
    // Handle edge cases.
    if (targetTickCount < 2) {
        targetTickCount = 2;
    }

    // Zero or near-zero range: expand to +-0.5.
    if (std::abs(max - min) < 1e-15) {
        min -= 0.5;
        max += 0.5;
    }

    // Ensure min < max.
    if (min > max) {
        std::swap(min, max);
    }

    const double range = max - min;
    const double roughSpacing = range / static_cast<double>(targetTickCount - 1);
    const double spacing = niceNum(roughSpacing, false);

    // Guard against zero spacing (should not happen after range fix, but be safe).
    if (spacing <= 0.0) {
        return TickResult{{min, max}, 0, max - min};
    }

    const double niceMin = std::floor(min / spacing) * spacing;
    const double niceMax = std::ceil(max / spacing) * spacing;

    // Determine decimal places needed for the spacing.
    int decimalPlaces = 0;
    if (spacing < 1.0) {
        decimalPlaces = static_cast<int>(std::ceil(-std::log10(spacing)));
        // Ensure at least enough precision.
        if (decimalPlaces < 0) {
            decimalPlaces = 0;
        }
    }

    // Also check if min values need more decimal places.
    if (std::abs(niceMin) > 0.0 && spacing < 1.0) {
        const int minDecimals = static_cast<int>(
            std::ceil(-std::log10(spacing)));
        decimalPlaces = std::max(decimalPlaces, minDecimals);
    }

    // Generate tick values.
    std::vector<double> values;
    // Use a count-based approach to avoid floating-point accumulation errors.
    const int tickCount = static_cast<int>(std::round((niceMax - niceMin) / spacing)) + 1;
    values.reserve(static_cast<size_t>(tickCount));

    for (int i = 0; i < tickCount; ++i) {
        const double tick = niceMin + static_cast<double>(i) * spacing;
        values.push_back(tick);
    }

    return TickResult{std::move(values), decimalPlaces, spacing};
}

} // namespace lumen::plot
