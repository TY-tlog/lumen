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

/// A histogram plot item that bins data from a Rank1Dataset and draws bars.
///
/// Supports automatic bin width calculation via Scott, Freedman-Diaconis,
/// and Sturges rules. Normalization modes: Count, Density, Probability.
class HistogramSeries : public PlotItem {
public:
    enum class BinRule { Scott, FreedmanDiaconis, Sturges };
    enum class Normalization { Count, Density, Probability };

    /// A single histogram bin.
    struct Bin {
        double lo;     ///< Left edge of the bin.
        double hi;     ///< Right edge of the bin.
        double value;  ///< Count, density, or probability depending on normalization.
    };

    /// Construct a histogram from a Rank1Dataset (must be double type).
    /// @throws std::invalid_argument if data is null or not double type.
    explicit HistogramSeries(std::shared_ptr<data::Rank1Dataset> data);

    // --- PlotItem overrides ---
    Type type() const override { return Type::Histogram; }
    QRectF dataBounds() const override;
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    QColor primaryColor() const override { return fillColor_; }

    // --- Histogram-specific API ---

    /// Set the number of bins manually.
    void setBinCount(int n);

    /// Use automatic bin width rule.
    void setAutoBinning(BinRule rule);

    /// Set normalization mode.
    void setNormalization(Normalization n);

    /// Fill color for the bars.
    void setFillColor(QColor color);

    /// Outline color for the bars.
    void setOutlineColor(QColor color);

    void setName(QString name);
    void setVisible(bool visible);

    // Getters
    [[nodiscard]] int binCount() const { return binCount_; }
    [[nodiscard]] BinRule binRule() const { return binRule_; }
    [[nodiscard]] Normalization normalization() const { return normalization_; }
    [[nodiscard]] QColor fillColor() const { return fillColor_; }
    [[nodiscard]] QColor outlineColor() const { return outlineColor_; }

    /// Compute bins from data. Exposed for testing.
    [[nodiscard]] std::vector<Bin> computeBins() const;

    /// Compute bin count from the current rule and data. Exposed for testing.
    [[nodiscard]] int computeAutoBinCount() const;

private:
    std::shared_ptr<data::Rank1Dataset> data_;
    int binCount_ = 0;         ///< 0 means use auto binning.
    BinRule binRule_ = BinRule::Sturges;
    Normalization normalization_ = Normalization::Count;
    QColor fillColor_ = QColor(70, 130, 180); // steel blue
    QColor outlineColor_ = Qt::black;
    QString name_;
    bool visible_ = true;
};

} // namespace lumen::plot
