#pragma once

#include "plot/PlotItem.h"

#include <QString>

#include <memory>

namespace lumen::data {
class Rank1Dataset;
} // namespace lumen::data

namespace lumen::plot {

/// A bar chart series referencing X and Y Rank1Datasets.
///
/// Draws vertical bars from the y=0 baseline to each data point.
/// Bar width is relative to median X spacing. Both datasets must contain double data.
class BarSeries : public PlotItem {
public:
    /// Construct a bar series from X and Y Rank1Datasets.
    /// Both datasets must contain double data.
    /// @throws std::invalid_argument if datasets are null, not double type, or have different row counts.
    BarSeries(std::shared_ptr<data::Rank1Dataset> xDs, std::shared_ptr<data::Rank1Dataset> yDs,
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
    [[nodiscard]] const std::shared_ptr<data::Rank1Dataset>& xDataset() const { return xDs_; }
    [[nodiscard]] const std::shared_ptr<data::Rank1Dataset>& yDataset() const { return yDs_; }

private:
    double computeMedianXSpacing() const;

    std::shared_ptr<data::Rank1Dataset> xDs_;
    std::shared_ptr<data::Rank1Dataset> yDs_;
    QColor fillColor_;
    QColor outlineColor_ = Qt::transparent;
    double barWidth_ = 0.8;
    QString name_;
    bool visible_ = true;
    mutable double cachedMedianSpacing_ = -1.0;
};

} // namespace lumen::plot
