#pragma once

#include "plot/Annotation.h"

#include <QFont>

namespace lumen::plot {

/// Free text annotation at a position with optional rotation.
/// Supports inline LaTeX via $...$ syntax (renders as plain text
/// until MicroTeX is integrated in Phase 9.4).
class TextAnnotation : public Annotation {
public:
    TextAnnotation(const QString& text, QPointF position,
                   Anchor anchor = Anchor::Data);

    Type type() const override { return Type::Text; }
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const override;
    QRectF boundingRect(const CoordinateMapper& mapper,
                        const QRectF& plotArea) const override;
    QJsonObject toJson() const override;
    static std::unique_ptr<Annotation> fromJson(const QJsonObject& obj);

    [[nodiscard]] QString text() const { return text_; }
    void setText(const QString& t) { text_ = t; }
    [[nodiscard]] QPointF position() const { return position_; }
    void setPosition(QPointF p) { position_ = p; }
    [[nodiscard]] QColor color() const { return color_; }
    void setColor(QColor c) { color_ = c; }
    [[nodiscard]] double rotation() const { return rotation_; }
    void setRotation(double deg) { rotation_ = deg; }
    [[nodiscard]] int fontSize() const { return fontSize_; }
    void setFontSize(int s) { fontSize_ = s; }

private:
    QString text_;
    QPointF position_;
    QColor color_ = Qt::black;
    double rotation_ = 0.0;
    int fontSize_ = 11;
};

}  // namespace lumen::plot
