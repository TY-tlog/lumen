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

/// A violin plot item that renders a KDE (kernel density estimate)
/// mirrored about a center line.
///
/// Uses a Gaussian kernel with either manual or Silverman auto-bandwidth.
/// Evaluates the KDE at 100 points between data min and max.
class ViolinSeries : public PlotItem {
public:
    /// A single KDE evaluation point.
    struct KdePoint {
        double y;       ///< Data value (vertical axis).
        double density;  ///< Estimated density at this point.
    };

    /// Construct a violin plot from a Rank1Dataset (must be double type).
    /// @throws std::invalid_argument if data is null or not double type.
    explicit ViolinSeries(std::shared_ptr<data::Rank1Dataset> data);

    // --- PlotItem overrides ---
    Type type() const override { return Type::Violin; }
    QRectF dataBounds() const override;
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    QColor primaryColor() const override { return fillColor_; }

    // --- Violin-specific API ---

    /// Set a manual KDE bandwidth.
    void setKdeBandwidth(double h);

    /// Enable or disable Silverman auto-bandwidth.
    void setKdeBandwidthAuto(bool automatic);

    /// Enable split violin (draw only one side).
    void setSplit(bool split);

    /// Fill color for the violin.
    void setFillColor(QColor color);

    /// Outline color.
    void setOutlineColor(QColor color);

    /// X position of the center line.
    void setPosition(double x);

    /// Maximum half-width of the violin in data units.
    void setMaxWidth(double w);

    void setName(QString name);
    void setVisible(bool visible);

    // Getters
    [[nodiscard]] double kdeBandwidth() const { return bandwidth_; }
    [[nodiscard]] bool kdeBandwidthAuto() const { return autoBandwidth_; }
    [[nodiscard]] bool split() const { return split_; }
    [[nodiscard]] QColor fillColor() const { return fillColor_; }
    [[nodiscard]] QColor outlineColor() const { return outlineColor_; }
    [[nodiscard]] double position() const { return position_; }
    [[nodiscard]] double maxWidth() const { return maxWidth_; }

    /// Compute KDE points. Exposed for testing.
    [[nodiscard]] std::vector<KdePoint> computeKde() const;

    /// Compute Silverman bandwidth. Exposed for testing.
    [[nodiscard]] double computeSilvermanBandwidth() const;

private:
    std::shared_ptr<data::Rank1Dataset> data_;
    double bandwidth_ = 1.0;
    bool autoBandwidth_ = true;
    bool split_ = false;
    QColor fillColor_ = QColor(70, 130, 180);
    QColor outlineColor_ = Qt::black;
    double position_ = 0.0;
    double maxWidth_ = 0.5;
    QString name_;
    bool visible_ = true;
};

} // namespace lumen::plot
