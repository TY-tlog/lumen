#include "TextAnnotation.h"

#include "plot/CoordinateMapper.h"

#include <QFontMetricsF>
#include <QJsonArray>
#include <QPainter>

namespace lumen::plot {

TextAnnotation::TextAnnotation(const QString& text, QPointF position, Anchor anchor)
    : text_(text)
    , position_(position)
{
    anchor_ = anchor;
}

void TextAnnotation::paint(QPainter* painter, const CoordinateMapper& mapper,
                            const QRectF& plotArea) const
{
    QPointF pix = resolvePoint(position_, anchor_, mapper, plotArea);

    QFont font = painter->font();
    font.setPixelSize(fontSize_);
    painter->setFont(font);
    painter->setPen(color_);

    painter->save();
    painter->translate(pix);
    if (std::abs(rotation_) > 0.01)
        painter->rotate(rotation_);

    painter->drawText(0, 0, text_);
    painter->restore();
}

QRectF TextAnnotation::boundingRect(const CoordinateMapper& mapper,
                                     const QRectF& plotArea) const
{
    QPointF pix = resolvePoint(position_, anchor_, mapper, plotArea);
    QFont font;
    font.setPixelSize(fontSize_);
    QFontMetricsF fm(font);
    QRectF textRect = fm.boundingRect(text_);
    textRect.moveTopLeft(pix);
    return textRect.adjusted(-3, -3, 3, 3);
}

QJsonObject TextAnnotation::toJson() const
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("text");
    obj[QStringLiteral("anchor")] = static_cast<int>(anchor_);
    obj[QStringLiteral("text")] = text_;
    obj[QStringLiteral("position")] = QJsonArray{position_.x(), position_.y()};
    obj[QStringLiteral("color")] = color_.name();
    obj[QStringLiteral("rotation")] = rotation_;
    obj[QStringLiteral("fontSize")] = fontSize_;
    obj[QStringLiteral("visible")] = visible_;
    obj[QStringLiteral("name")] = name_;
    return obj;
}

std::unique_ptr<Annotation> TextAnnotation::fromJson(const QJsonObject& obj)
{
    QJsonArray posArr = obj[QStringLiteral("position")].toArray();
    auto anchor = static_cast<Anchor>(obj[QStringLiteral("anchor")].toInt());

    auto ann = std::make_unique<TextAnnotation>(
        obj[QStringLiteral("text")].toString(),
        QPointF(posArr[0].toDouble(), posArr[1].toDouble()),
        anchor);

    ann->setColor(QColor(obj[QStringLiteral("color")].toString()));
    ann->setRotation(obj[QStringLiteral("rotation")].toDouble());
    ann->setFontSize(obj[QStringLiteral("fontSize")].toInt(11));
    ann->setVisible(obj[QStringLiteral("visible")].toBool(true));
    ann->setName(obj[QStringLiteral("name")].toString());
    return ann;
}

}  // namespace lumen::plot
