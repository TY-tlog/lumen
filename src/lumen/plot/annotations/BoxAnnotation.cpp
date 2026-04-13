#include "BoxAnnotation.h"

#include "plot/CoordinateMapper.h"

#include <QJsonArray>
#include <QPainter>

namespace lumen::plot {

BoxAnnotation::BoxAnnotation(QPointF topLeft, QPointF bottomRight, Anchor anchor)
    : topLeft_(topLeft)
    , bottomRight_(bottomRight)
{
    anchor_ = anchor;
}

void BoxAnnotation::paint(QPainter* painter, const CoordinateMapper& mapper,
                           const QRectF& plotArea) const
{
    QPointF pTL = resolvePoint(topLeft_, anchor_, mapper, plotArea);
    QPointF pBR = resolvePoint(bottomRight_, anchor_, mapper, plotArea);
    QRectF rect = QRectF(pTL, pBR).normalized();

    painter->setBrush(fillColor_);
    painter->setPen(QPen(borderColor_, borderWidth_));
    painter->drawRect(rect);
}

QRectF BoxAnnotation::boundingRect(const CoordinateMapper& mapper,
                                    const QRectF& plotArea) const
{
    QPointF pTL = resolvePoint(topLeft_, anchor_, mapper, plotArea);
    QPointF pBR = resolvePoint(bottomRight_, anchor_, mapper, plotArea);
    return QRectF(pTL, pBR).normalized().adjusted(-2, -2, 2, 2);
}

QJsonObject BoxAnnotation::toJson() const
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("box");
    obj[QStringLiteral("anchor")] = static_cast<int>(anchor_);
    obj[QStringLiteral("topLeft")] = QJsonArray{topLeft_.x(), topLeft_.y()};
    obj[QStringLiteral("bottomRight")] = QJsonArray{bottomRight_.x(), bottomRight_.y()};
    obj[QStringLiteral("fillColor")] = fillColor_.name(QColor::HexArgb);
    obj[QStringLiteral("borderColor")] = borderColor_.name();
    obj[QStringLiteral("borderWidth")] = borderWidth_;
    obj[QStringLiteral("visible")] = visible_;
    obj[QStringLiteral("name")] = name_;
    return obj;
}

std::unique_ptr<Annotation> BoxAnnotation::fromJson(const QJsonObject& obj)
{
    QJsonArray tlArr = obj[QStringLiteral("topLeft")].toArray();
    QJsonArray brArr = obj[QStringLiteral("bottomRight")].toArray();
    auto anchor = static_cast<Anchor>(obj[QStringLiteral("anchor")].toInt());

    auto ann = std::make_unique<BoxAnnotation>(
        QPointF(tlArr[0].toDouble(), tlArr[1].toDouble()),
        QPointF(brArr[0].toDouble(), brArr[1].toDouble()),
        anchor);

    ann->setFillColor(QColor(obj[QStringLiteral("fillColor")].toString()));
    ann->setBorderColor(QColor(obj[QStringLiteral("borderColor")].toString()));
    ann->setBorderWidth(obj[QStringLiteral("borderWidth")].toDouble(1.0));
    ann->setVisible(obj[QStringLiteral("visible")].toBool(true));
    ann->setName(obj[QStringLiteral("name")].toString());
    return ann;
}

}  // namespace lumen::plot
