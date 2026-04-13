#pragma once

#include "plot/Annotation.h"

namespace lumen::plot {

/// Scale bar showing a physical length with a label.
/// Common in microscopy figures. Auto-positioned by default, draggable.
class ScaleBar : public Annotation {
public:
    ScaleBar(double lengthInDataUnits, const QString& unitLabel,
             QPointF position = QPointF(0.8, 0.05),
             Anchor anchor = Anchor::AxisFraction);

    Type type() const override { return Type::ScaleBar; }
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    QRectF boundingRect(const CoordinateMapper& mapper,
                        const QRectF& plotArea) const override;
    QJsonObject toJson() const override;
    static std::unique_ptr<Annotation> fromJson(const QJsonObject& obj);

    [[nodiscard]] double lengthInDataUnits() const { return lengthData_; }
    void setLengthInDataUnits(double l) { lengthData_ = l; }
    [[nodiscard]] QString unitLabel() const { return unitLabel_; }
    void setUnitLabel(const QString& u) { unitLabel_ = u; }
    [[nodiscard]] QColor color() const { return color_; }
    void setColor(QColor c) { color_ = c; }
    [[nodiscard]] double barHeight() const { return barHeight_; }
    void setBarHeight(double h) { barHeight_ = h; }

private:
    double lengthData_;
    QString unitLabel_;
    QPointF position_;
    QColor color_ = Qt::black;
    double barHeight_ = 4.0;
};

}  // namespace lumen::plot
