#include "plot/Axis.h"

#include "plot/LineSeries.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumen::plot {

Axis::Axis(AxisOrientation orientation)
    : orientation_(orientation) {}

void Axis::setRange(double min, double max) {
    min_ = min;
    max_ = max;
}

void Axis::setLabel(const QString& label) {
    label_ = label;
}

void Axis::autoRange(const std::vector<LineSeries>& seriesList) {
    if (seriesList.empty()) {
        min_ = 0.0;
        max_ = 1.0;
        return;
    }

    double lo = std::numeric_limits<double>::max();
    double hi = std::numeric_limits<double>::lowest();

    for (const auto& series : seriesList) {
        auto range = series.dataRange();
        if (orientation_ == AxisOrientation::Horizontal) {
            lo = std::min(lo, range.xMin);
            hi = std::max(hi, range.xMax);
        } else {
            lo = std::min(lo, range.yMin);
            hi = std::max(hi, range.yMax);
        }
    }

    if (lo >= hi) {
        lo -= 0.5;
        hi += 0.5;
    } else {
        double padding = (hi - lo) * 0.05;
        lo -= padding;
        hi += padding;
    }

    min_ = lo;
    max_ = hi;
}

std::vector<TickMark> Axis::ticks() const {
    auto result = NiceNumbers::compute(min_, max_);
    std::vector<TickMark> marks;
    marks.reserve(result.values.size());
    for (double v : result.values) {
        marks.push_back({v, QString::number(v, 'f', result.decimalPlaces)});
    }
    return marks;
}

}  // namespace lumen::plot
