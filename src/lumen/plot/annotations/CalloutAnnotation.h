#pragma once

#include "plot/Annotation.h"

namespace lumen::plot {

/// Callout: box with text and an arrow pointing to a data point.
class CalloutAnnotation : public Annotation {
public:
    CalloutAnnotation(const QString& text, QPointF boxPosition,
                      QPointF targetPoint, Anchor anchor = Anchor::Data);

    Type type() const override { return Type::Callout; }
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    QRectF boundingRect(const CoordinateMapper& mapper,
                        const QRectF& plotArea) const override;
    QJsonObject toJson() const override;
    static std::unique_ptr<Annotation> fromJson(const QJsonObject& obj);

    [[nodiscard]] QString text() const { return text_; }
    void setText(const QString& t) { text_ = t; }
    [[nodiscard]] QPointF boxPosition() const { return boxPos_; }
    [[nodiscard]] QPointF targetPoint() const { return targetPt_; }
    [[nodiscard]] QColor fillColor() const { return fillColor_; }
    void setFillColor(QColor c) { fillColor_ = c; }
    [[nodiscard]] QColor textColor() const { return textColor_; }
    void setTextColor(QColor c) { textColor_ = c; }

private:
    QString text_;
    QPointF boxPos_;
    QPointF targetPt_;
    QColor fillColor_ = QColor(255, 255, 230);
    QColor textColor_ = Qt::black;
};

}  // namespace lumen::plot
