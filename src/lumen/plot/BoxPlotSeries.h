#pragma once

#include "plot/PlotItem.h"

#include <QColor>
#include <QPainter>
#include <QRectF>
#include <QString>

#include <memory>
#include <vector>

namespace lumen::data {
class Rank1Dataset;
} // namespace lumen::data

namespace lumen::plot {

class CoordinateMapper;

/// A box plot item that computes and renders box-and-whisker statistics
/// from a Rank1Dataset.
///
/// Computes: median, Q1, Q3, whiskers (Tukey, MinMax, or Percentile),
/// and outliers. Renders as box with median line, whiskers, and outlier dots.
class BoxPlotSeries : public PlotItem {
public:
    enum class WhiskerRule { Tukey, MinMax, Percentile };

    /// Computed statistics for the box plot.
    struct Stats {
        double median;
        double q1;
        double q3;
        double whiskerLo;
        double whiskerHi;
        std::vector<double> outliers;
    };

    /// Construct a box plot from a Rank1Dataset (must be double type).
    /// @throws std::invalid_argument if data is null or not double type.
    explicit BoxPlotSeries(std::shared_ptr<data::Rank1Dataset> data);

    // --- PlotItem overrides ---
    Type type() const override { return Type::BoxPlot; }
    QRectF dataBounds() const override;
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    QColor primaryColor() const override { return fillColor_; }

    // --- Box plot API ---

    void setWhiskerRule(WhiskerRule rule);
    void setNotched(bool notched);
    void setOutliersVisible(bool visible);
    void setFillColor(QColor color);
    void setOutlineColor(QColor color);
    void setPosition(double x);  ///< X position of the box center.
    void setBoxWidth(double w);  ///< Width of the box in data units.
    void setName(QString name);
    void setVisible(bool visible);

    // Getters
    [[nodiscard]] WhiskerRule whiskerRule() const { return whiskerRule_; }
    [[nodiscard]] bool notched() const { return notched_; }
    [[nodiscard]] bool outliersVisible() const { return outliersVisible_; }
    [[nodiscard]] QColor fillColor() const { return fillColor_; }
    [[nodiscard]] QColor outlineColor() const { return outlineColor_; }
    [[nodiscard]] double position() const { return position_; }
    [[nodiscard]] double boxWidth() const { return boxWidth_; }

    /// Compute statistics from the data. Exposed for testing.
    [[nodiscard]] Stats computeStats() const;

private:
    std::shared_ptr<data::Rank1Dataset> data_;
    WhiskerRule whiskerRule_ = WhiskerRule::Tukey;
    bool notched_ = false;
    bool outliersVisible_ = true;
    QColor fillColor_ = QColor(70, 130, 180);
    QColor outlineColor_ = Qt::black;
    double position_ = 0.0;
    double boxWidth_ = 1.0;
    QString name_;
    bool visible_ = true;
};

} // namespace lumen::plot
