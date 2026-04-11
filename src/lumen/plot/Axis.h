#pragma once

#include "NiceNumbers.h"

#include <QObject>
#include <QString>

#include <vector>

namespace lumen::plot {

class LineSeries;

/// Axis orientation.
enum class AxisOrientation { Horizontal, Vertical };

/// Whether axis range is auto-computed or manually set.
enum class RangeMode { Auto, Manual };

/// Tick label formatting mode.
enum class TickFormat { Auto, Scientific, Fixed };

/// A single tick mark with its data value and formatted label.
struct TickMark {
    double value{0.0};
    QString label;
};

/// An axis for a 2D plot — manages range, tick generation, and label.
///
/// Emits changed() when any property is modified.
class Axis : public QObject {
    Q_OBJECT
public:
    explicit Axis(AxisOrientation orientation, QObject* parent = nullptr);

    /// Set the data range manually. Sets range mode to Manual.
    void setRange(double min, double max);

    /// Set the axis label text.
    void setLabel(const QString& label);

    /// Set range mode (Auto or Manual).
    void setRangeMode(RangeMode mode);

    /// Set manual range bounds (does not change range mode).
    void setManualRange(double min, double max);

    /// Set desired tick count. 0 = auto (uses NiceNumbers default).
    void setTickCount(int count);

    /// Set tick label format.
    void setTickFormat(TickFormat format);

    /// Set decimal places for Fixed tick format.
    void setTickFormatDecimals(int n);

    /// Set grid line visibility.
    void setGridVisible(bool visible);

    /// Auto-range from a set of line series. Adds 5% padding.
    /// For a horizontal axis, uses the X range; for vertical, the Y range.
    /// Sets range mode to Auto.
    void autoRange(const std::vector<LineSeries>& seriesList);

    /// Extend the current auto-range to include additional bounds.
    /// Only meaningful after autoRange() has been called. Keeps Auto mode.
    void extendAutoRange(double lo, double hi);

    /// Set the auto-range values directly and switch to Auto mode.
    void setAutoRangeValues(double lo, double hi);

    /// Current minimum value (auto or manual depending on range mode).
    [[nodiscard]] double min() const;

    /// Current maximum value (auto or manual depending on range mode).
    [[nodiscard]] double max() const;

    /// Axis label text.
    [[nodiscard]] const QString& label() const { return label_; }

    /// Axis orientation.
    [[nodiscard]] AxisOrientation orientation() const { return orientation_; }

    /// Current range mode.
    [[nodiscard]] RangeMode rangeMode() const { return rangeMode_; }

    /// Desired tick count (0 = auto).
    [[nodiscard]] int tickCount() const { return tickCount_; }

    /// Tick label format.
    [[nodiscard]] TickFormat tickFormat() const { return tickFormat_; }

    /// Decimal places for Fixed format.
    [[nodiscard]] int tickFormatDecimals() const { return tickFormatDecimals_; }

    /// Whether grid lines are visible for this axis.
    [[nodiscard]] bool gridVisible() const { return gridVisible_; }

    /// Manual range minimum.
    [[nodiscard]] double manualMin() const { return manualMin_; }

    /// Manual range maximum.
    [[nodiscard]] double manualMax() const { return manualMax_; }

    /// Compute tick marks using the NiceNumbers algorithm.
    /// Respects tickCount and tickFormat settings.
    [[nodiscard]] std::vector<TickMark> ticks() const;

signals:
    /// Emitted when any property changes.
    void changed();

private:
    AxisOrientation orientation_;
    double autoMin_ = 0.0;
    double autoMax_ = 1.0;
    double manualMin_ = 0.0;
    double manualMax_ = 1.0;
    QString label_;
    RangeMode rangeMode_ = RangeMode::Auto;
    int tickCount_ = 0;
    TickFormat tickFormat_ = TickFormat::Auto;
    int tickFormatDecimals_ = 2;
    bool gridVisible_ = true;

    /// Format a tick value according to current tickFormat settings.
    [[nodiscard]] QString formatTickValue(double value, int autoDecimalPlaces) const;
};

}  // namespace lumen::plot
