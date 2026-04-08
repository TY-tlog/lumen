#pragma once

#include "NiceNumbers.h"

#include <QString>

#include <vector>

namespace lumen::plot {

class LineSeries;

/// Axis orientation.
enum class AxisOrientation { Horizontal, Vertical };

/// A single tick mark with its data value and formatted label.
struct TickMark {
    double value{0.0};
    QString label;
};

/// An axis for a 2D plot — manages range, tick generation, and label.
class Axis {
public:
    explicit Axis(AxisOrientation orientation);

    /// Set the data range manually.
    void setRange(double min, double max);

    /// Set the axis label text.
    void setLabel(const QString& label);

    /// Auto-range from a set of line series. Adds 5% padding.
    /// For a horizontal axis, uses the X range; for vertical, the Y range.
    void autoRange(const std::vector<LineSeries>& seriesList);

    /// Current minimum value.
    [[nodiscard]] double min() const { return min_; }

    /// Current maximum value.
    [[nodiscard]] double max() const { return max_; }

    /// Axis label text.
    [[nodiscard]] const QString& label() const { return label_; }

    /// Axis orientation.
    [[nodiscard]] AxisOrientation orientation() const { return orientation_; }

    /// Compute tick marks using the NiceNumbers algorithm.
    [[nodiscard]] std::vector<TickMark> ticks() const;

private:
    AxisOrientation orientation_;
    double min_ = 0.0;
    double max_ = 1.0;
    QString label_;
};

}  // namespace lumen::plot
