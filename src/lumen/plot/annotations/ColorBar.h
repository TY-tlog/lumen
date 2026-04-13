#pragma once

#include "plot/Annotation.h"
#include "plot/Colormap.h"

namespace lumen::plot {

/// Color bar annotation for colormapped PlotItems (Heatmap, etc.).
/// Draws a vertical or horizontal gradient with tick labels.
class ColorBar : public Annotation {
public:
    enum class Orientation { Vertical, Horizontal };

    ColorBar(Colormap colormap, double valueMin, double valueMax,
             QPointF position = QPointF(0.92, 0.1),
             Anchor anchor = Anchor::AxisFraction);

    Type type() const override { return Type::ColorBar; }
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    QRectF boundingRect(const CoordinateMapper& mapper,
                        const QRectF& plotArea) const override;
    QJsonObject toJson() const override;
    static std::unique_ptr<Annotation> fromJson(const QJsonObject& obj);

    [[nodiscard]] const Colormap& colormap() const { return colormap_; }
    void setColormap(Colormap cm) { colormap_ = std::move(cm); }
    [[nodiscard]] double valueMin() const { return valueMin_; }
    [[nodiscard]] double valueMax() const { return valueMax_; }
    void setValueRange(double min, double max) { valueMin_ = min; valueMax_ = max; }
    [[nodiscard]] Orientation orientation() const { return orientation_; }
    void setOrientation(Orientation o) { orientation_ = o; }
    [[nodiscard]] QString unitLabel() const { return unitLabel_; }
    void setUnitLabel(const QString& u) { unitLabel_ = u; }

private:
    Colormap colormap_;
    double valueMin_;
    double valueMax_;
    QPointF position_;
    Orientation orientation_ = Orientation::Vertical;
    QString unitLabel_;
    double barWidth_ = 20.0;
    double barLength_ = 200.0;
};

}  // namespace lumen::plot
