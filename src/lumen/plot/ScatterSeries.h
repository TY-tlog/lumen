#pragma once

#include "plot/PlotItem.h"

#include <QString>

namespace lumen::data {
class Column;
} // namespace lumen::data

namespace lumen::plot {

enum class MarkerShape { Circle, Square, Triangle, Diamond, Plus, Cross };

/// A scatter plot series referencing X and Y columns from a DataFrame.
///
/// Draws individual markers at each data point, skipping NaN values.
/// Both columns must be of type Double.
class ScatterSeries : public PlotItem {
public:
    /// Construct a scatter series from X and Y columns.
    /// Both columns must be Double type.
    /// @throws std::invalid_argument if columns are not Double type or have different row counts.
    ScatterSeries(const data::Column* xCol, const data::Column* yCol,
                  QColor color, QString name = {});

    // --- PlotItem overrides ---
    Type type() const override { return Type::Scatter; }
    QRectF dataBounds() const override;
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    QColor primaryColor() const override { return color_; }

    // Setters
    void setColor(QColor c);
    void setMarkerShape(MarkerShape shape);
    void setMarkerSize(int pixels);  // clamped to 3-20
    void setFilled(bool filled);
    void setName(QString name);
    void setVisible(bool visible);

    // Getters
    [[nodiscard]] QColor color() const { return color_; }
    [[nodiscard]] MarkerShape markerShape() const { return shape_; }
    [[nodiscard]] int markerSize() const { return markerSize_; }
    [[nodiscard]] bool filled() const { return filled_; }
    [[nodiscard]] const data::Column* xColumn() const { return xCol_; }
    [[nodiscard]] const data::Column* yColumn() const { return yCol_; }

private:
    void drawMarker(QPainter* painter, QPointF center) const;

    const data::Column* xCol_;
    const data::Column* yCol_;
    QColor color_;
    MarkerShape shape_ = MarkerShape::Circle;
    int markerSize_ = 6;
    bool filled_ = true;
    QString name_;
    bool visible_ = true;
};

} // namespace lumen::plot
