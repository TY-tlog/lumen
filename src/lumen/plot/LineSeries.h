#pragma once

#include "plot/PlotItem.h"
#include "plot/PlotStyle.h"

#include <QPolygonF>
#include <QString>

#include <memory>
#include <vector>

namespace lumen::data {
class Rank1Dataset;
} // namespace lumen::data

namespace lumen::plot {

/// Data range of a series (min/max of X and Y, excluding NaN).
struct DataRange {
    double xMin{0.0};
    double xMax{0.0};
    double yMin{0.0};
    double yMax{0.0};
};

/// A line series referencing X and Y Rank1Datasets.
///
/// Builds polylines from the dataset data, breaking at NaN values.
/// Both datasets must contain double data.
class LineSeries : public PlotItem {
public:
    /// Construct a line series from X and Y Rank1Datasets.
    /// Both datasets must contain double data.
    /// @throws std::invalid_argument if datasets are null, not double type, or have different row counts.
    LineSeries(std::shared_ptr<data::Rank1Dataset> xDs, std::shared_ptr<data::Rank1Dataset> yDs,
               PlotStyle style, QString name = {});

    // --- PlotItem overrides ---
    Type type() const override { return Type::Line; }
    QRectF dataBounds() const override;
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    QColor primaryColor() const override { return style_.color; }

    /// Build polylines from the dataset data.
    /// Returns one QPolygonF per contiguous non-NaN segment.
    [[nodiscard]] std::vector<QPolygonF> buildPolylines() const;

    /// Compute the data range (min/max of X and Y, ignoring NaN).
    [[nodiscard]] DataRange dataRange() const;

    [[nodiscard]] const PlotStyle& style() const { return style_; }

    /// Access the X dataset.
    [[nodiscard]] const std::shared_ptr<data::Rank1Dataset>& xDataset() const { return xDs_; }
    /// Access the Y dataset.
    [[nodiscard]] const std::shared_ptr<data::Rank1Dataset>& yDataset() const { return yDs_; }

    void setStyle(PlotStyle style);
    void setName(QString name);
    void setVisible(bool visible);

private:
    std::shared_ptr<data::Rank1Dataset> xDs_;
    std::shared_ptr<data::Rank1Dataset> yDs_;
    PlotStyle style_;
    QString name_;
    bool visible_ = true;
};

} // namespace lumen::plot
