#pragma once

#include "Axis.h"
#include "LineSeries.h"
#include "ViewTransform.h"

#include <QFontMetrics>
#include <QRectF>
#include <QSizeF>
#include <QString>

#include <vector>

namespace lumen::plot {

/// Margin values for the four sides of the plot area.
struct PlotMargins {
    double left = 0.0;
    double top = 0.0;
    double right = 0.0;
    double bottom = 0.0;
};

/// Top-level plot container — owns axes, series, title.
///
/// Given a widget size, computes the plot area rectangle accounting
/// for margins needed by axis labels, tick labels, and title.
class PlotScene {
public:
    PlotScene();

    // --- Series management ---
    void addSeries(LineSeries series);
    void clearSeries();
    [[nodiscard]] const std::vector<LineSeries>& series() const { return series_; }
    [[nodiscard]] std::size_t seriesCount() const { return series_.size(); }

    /// Mutable access to a series by index (for editing via commands).
    /// @throws std::out_of_range if index >= seriesCount().
    LineSeries& seriesAt(std::size_t index);

    // --- Axes ---
    [[nodiscard]] Axis& xAxis() { return xAxis_; }
    [[nodiscard]] Axis& yAxis() { return yAxis_; }
    [[nodiscard]] const Axis& xAxis() const { return xAxis_; }
    [[nodiscard]] const Axis& yAxis() const { return yAxis_; }

    // --- View transform ---
    [[nodiscard]] ViewTransform& viewTransform() { return viewTransform_; }
    [[nodiscard]] const ViewTransform& viewTransform() const { return viewTransform_; }

    // --- Title ---
    void setTitle(const QString& title);
    [[nodiscard]] const QString& title() const { return title_; }

    /// Compute the data plotting area given the total widget size.
    /// Accounts for margins: left (Y ticks+label), bottom (X ticks+label),
    /// top (title), right (padding).
    [[nodiscard]] QRectF computePlotArea(QSizeF widgetSize) const;

    /// Compute dynamic margins based on tick labels, axis labels, and title.
    /// Uses the actual font metrics to determine precise sizes.
    [[nodiscard]] PlotMargins computeMargins(const QFontMetrics& tickFm,
                                             const QFontMetrics& labelFm,
                                             const QFontMetrics& titleFm) const;

    /// Auto-range both axes from all series data and sync ViewTransform.
    void autoRange();

private:
    Axis xAxis_{AxisOrientation::Horizontal};
    Axis yAxis_{AxisOrientation::Vertical};
    ViewTransform viewTransform_;
    std::vector<LineSeries> series_;
    QString title_;

    /// Cached margins for 1-pixel debounce (prevents jiggle during live edits).
    mutable PlotMargins cachedMargins_{};
    mutable bool hasCachedMargins_ = false;
};

}  // namespace lumen::plot
