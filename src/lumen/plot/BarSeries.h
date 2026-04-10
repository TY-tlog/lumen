#pragma once

#include "plot/PlotItem.h"

#include <QString>

namespace lumen::data {
class Column;
} // namespace lumen::data

namespace lumen::plot {

/// A bar chart series referencing X and Y columns from a DataFrame.
///
/// Draws vertical bars from the y=0 baseline to each data point.
/// Bar width is relative to median X spacing. Both columns must be Double type.
class BarSeries : public PlotItem {
public:
    /// Construct a bar series from X and Y columns.
    /// Both columns must be Double type.
    /// @throws std::invalid_argument if columns are not Double type or have different row counts.
    BarSeries(const data::Column* xCol, const data::Column* yCol,
              QColor fillColor, QString name = {});

    // --- PlotItem overrides ---
    Type type() const override { return Type::Bar; }
    QRectF dataBounds() const override;  // includes y=0 baseline
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    QColor primaryColor() const override { return fillColor_; }

    // Setters
    void setFillColor(QColor c);
    void setOutlineColor(QColor c);  // transparent = none
    void setBarWidth(double relative);  // clamped to 0.1-1.0
    void setName(QString name);
    void setVisible(bool visible);

    // Getters
    [[nodiscard]] QColor fillColor() const { return fillColor_; }
    [[nodiscard]] QColor outlineColor() const { return outlineColor_; }
    [[nodiscard]] double barWidth() const { return barWidth_; }
    [[nodiscard]] const data::Column* xColumn() const { return xCol_; }
    [[nodiscard]] const data::Column* yColumn() const { return yCol_; }

private:
    double computeMedianXSpacing() const;

    const data::Column* xCol_;
    const data::Column* yCol_;
    QColor fillColor_;
    QColor outlineColor_ = Qt::transparent;
    double barWidth_ = 0.8;
    QString name_;
    bool visible_ = true;
    mutable double cachedMedianSpacing_ = -1.0;
};

} // namespace lumen::plot
