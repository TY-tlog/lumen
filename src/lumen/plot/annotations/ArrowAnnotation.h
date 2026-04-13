#pragma once

#include "plot/Annotation.h"

namespace lumen::plot {

/// Arrow from point A to point B with configurable head style.
class ArrowAnnotation : public Annotation {
public:
    enum class HeadStyle { Open, Filled, Double };

    ArrowAnnotation(QPointF from, QPointF to, Anchor anchor = Anchor::Data);

    Type type() const override { return Type::Arrow; }
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    QRectF boundingRect(const CoordinateMapper& mapper,
                        const QRectF& plotArea) const override;
    QJsonObject toJson() const override;
    static std::unique_ptr<Annotation> fromJson(const QJsonObject& obj);

    // Properties
    [[nodiscard]] QPointF from() const { return from_; }
    [[nodiscard]] QPointF to() const { return to_; }
    void setFrom(QPointF p) { from_ = p; }
    void setTo(QPointF p) { to_ = p; }

    [[nodiscard]] QColor color() const { return color_; }
    void setColor(QColor c) { color_ = c; }

    [[nodiscard]] double lineWidth() const { return lineWidth_; }
    void setLineWidth(double w) { lineWidth_ = w; }

    [[nodiscard]] HeadStyle headStyle() const { return headStyle_; }
    void setHeadStyle(HeadStyle s) { headStyle_ = s; }

private:
    QPointF from_;
    QPointF to_;
    QColor color_ = Qt::red;
    double lineWidth_ = 1.5;
    HeadStyle headStyle_ = HeadStyle::Filled;
};

}  // namespace lumen::plot
