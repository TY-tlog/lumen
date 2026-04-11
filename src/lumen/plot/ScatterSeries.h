#pragma once

#include "plot/PlotItem.h"

#include <QString>

#include <memory>

namespace lumen::data {
class Rank1Dataset;
} // namespace lumen::data

namespace lumen::plot {

enum class MarkerShape { Circle, Square, Triangle, Diamond, Plus, Cross };

/// A scatter plot series referencing X and Y Rank1Datasets.
///
/// Draws individual markers at each data point, skipping NaN values.
/// Both datasets must contain double data.
class ScatterSeries : public PlotItem {
public:
    /// Construct a scatter series from X and Y Rank1Datasets.
    /// Both datasets must contain double data.
    /// @throws std::invalid_argument if datasets are null, not double type, or have different row counts.
    ScatterSeries(std::shared_ptr<data::Rank1Dataset> xDs, std::shared_ptr<data::Rank1Dataset> yDs,
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
    [[nodiscard]] const std::shared_ptr<data::Rank1Dataset>& xDataset() const { return xDs_; }
    [[nodiscard]] const std::shared_ptr<data::Rank1Dataset>& yDataset() const { return yDs_; }

private:
    void drawMarker(QPainter* painter, QPointF center) const;

    std::shared_ptr<data::Rank1Dataset> xDs_;
    std::shared_ptr<data::Rank1Dataset> yDs_;
    QColor color_;
    MarkerShape shape_ = MarkerShape::Circle;
    int markerSize_ = 6;
    bool filled_ = true;
    QString name_;
    bool visible_ = true;
};

} // namespace lumen::plot
