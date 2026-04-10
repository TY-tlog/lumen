#include "plot/Axis.h"

#include "plot/LineSeries.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumen::plot {

Axis::Axis(AxisOrientation orientation, QObject* parent)
    : QObject(parent)
    , orientation_(orientation) {}

void Axis::setRange(double min, double max) {
    // Legacy behavior: setRange implies Manual mode.
    bool changed = (rangeMode_ != RangeMode::Manual) ||
                   (manualMin_ != min) || (manualMax_ != max);
    rangeMode_ = RangeMode::Manual;
    manualMin_ = min;
    manualMax_ = max;
    if (changed) {
        emit this->changed();
    }
}

void Axis::setLabel(const QString& label) {
    if (label_ != label) {
        label_ = label;
        emit changed();
    }
}

void Axis::setRangeMode(RangeMode mode) {
    if (rangeMode_ != mode) {
        rangeMode_ = mode;
        emit changed();
    }
}

void Axis::setManualRange(double min, double max) {
    if (manualMin_ != min || manualMax_ != max) {
        manualMin_ = min;
        manualMax_ = max;
        emit changed();
    }
}

void Axis::setTickCount(int count) {
    if (tickCount_ != count) {
        tickCount_ = count;
        emit changed();
    }
}

void Axis::setTickFormat(TickFormat format) {
    if (tickFormat_ != format) {
        tickFormat_ = format;
        emit changed();
    }
}

void Axis::setTickFormatDecimals(int n) {
    if (tickFormatDecimals_ != n) {
        tickFormatDecimals_ = n;
        emit changed();
    }
}

void Axis::setGridVisible(bool visible) {
    if (gridVisible_ != visible) {
        gridVisible_ = visible;
        emit changed();
    }
}

void Axis::autoRange(const std::vector<LineSeries>& seriesList) {
    rangeMode_ = RangeMode::Auto;

    if (seriesList.empty()) {
        autoMin_ = 0.0;
        autoMax_ = 1.0;
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

    autoMin_ = lo;
    autoMax_ = hi;
}

void Axis::extendAutoRange(double lo, double hi) {
    double padding = (hi > lo) ? (hi - lo) * 0.05 : 0.5;
    double paddedLo = lo - padding;
    double paddedHi = hi + padding;
    autoMin_ = std::min(autoMin_, paddedLo);
    autoMax_ = std::max(autoMax_, paddedHi);
}

double Axis::min() const {
    return (rangeMode_ == RangeMode::Manual) ? manualMin_ : autoMin_;
}

double Axis::max() const {
    return (rangeMode_ == RangeMode::Manual) ? manualMax_ : autoMax_;
}

QString Axis::formatTickValue(double value, int autoDecimalPlaces) const {
    switch (tickFormat_) {
        case TickFormat::Scientific:
            return QString::number(value, 'e', tickFormatDecimals_);
        case TickFormat::Fixed:
            return QString::number(value, 'f', tickFormatDecimals_);
        case TickFormat::Auto:
        default:
            return QString::number(value, 'f', autoDecimalPlaces);
    }
}

std::vector<TickMark> Axis::ticks() const {
    double lo = min();
    double hi = max();

    int targetTicks = (tickCount_ > 0) ? tickCount_ : 7;
    auto result = NiceNumbers::compute(lo, hi, targetTicks);

    std::vector<TickMark> marks;
    marks.reserve(result.values.size());
    for (double v : result.values) {
        marks.push_back({v, formatTickValue(v, result.decimalPlaces)});
    }
    return marks;
}

}  // namespace lumen::plot
