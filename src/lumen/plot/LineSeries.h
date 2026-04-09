#pragma once

#include "plot/PlotStyle.h"

#include <QPolygonF>
#include <QString>

#include <vector>

namespace lumen::data {
class Column;
} // namespace lumen::data

namespace lumen::plot {

/// Data range of a series (min/max of X and Y, excluding NaN).
struct DataRange {
    double xMin{0.0};
    double xMax{0.0};
    double yMin{0.0};
    double yMax{0.0};
};

/// A line series referencing X and Y columns from a DataFrame.
///
/// Builds polylines from the column data, breaking at NaN values.
/// Both columns must be of type Double.
class LineSeries {
public:
    /// Construct a line series from X and Y columns.
    /// Both columns must be Double type.
    /// @throws std::invalid_argument if columns are not Double type or have different row counts.
    LineSeries(const data::Column* xCol, const data::Column* yCol,
               PlotStyle style, QString name = {});

    /// Build polylines from the column data.
    /// Returns one QPolygonF per contiguous non-NaN segment.
    [[nodiscard]] std::vector<QPolygonF> buildPolylines() const;

    /// Compute the data range (min/max of X and Y, ignoring NaN).
    [[nodiscard]] DataRange dataRange() const;

    [[nodiscard]] const PlotStyle& style() const { return style_; }
    [[nodiscard]] const QString& name() const { return name_; }
    [[nodiscard]] bool isVisible() const { return visible_; }

    void setStyle(PlotStyle style);
    void setName(QString name);
    void setVisible(bool visible);

private:
    const data::Column* xCol_;
    const data::Column* yCol_;
    PlotStyle style_;
    QString name_;
    bool visible_ = true;
};

} // namespace lumen::plot
