#pragma once

#include "plot/Annotation.h"

namespace lumen::plot {

/// Rectangle annotation with fill, border, and dash style.
class BoxAnnotation : public Annotation {
public:
    BoxAnnotation(QPointF topLeft, QPointF bottomRight, Anchor anchor = Anchor::Data);

    Type type() const override { return Type::Box; }
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    QRectF boundingRect(const CoordinateMapper& mapper,
                        const QRectF& plotArea) const override;
    QJsonObject toJson() const override;
    static std::unique_ptr<Annotation> fromJson(const QJsonObject& obj);

    [[nodiscard]] QColor fillColor() const { return fillColor_; }
    void setFillColor(QColor c) { fillColor_ = c; }
    [[nodiscard]] QColor borderColor() const { return borderColor_; }
    void setBorderColor(QColor c) { borderColor_ = c; }
    [[nodiscard]] double borderWidth() const { return borderWidth_; }
    void setBorderWidth(double w) { borderWidth_ = w; }

private:
    QPointF topLeft_;
    QPointF bottomRight_;
    QColor fillColor_ = QColor(255, 255, 200, 128);
    QColor borderColor_ = Qt::black;
    double borderWidth_ = 1.0;
};

}  // namespace lumen::plot
