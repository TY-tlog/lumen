#include "CalloutAnnotation.h"

#include "plot/CoordinateMapper.h"

#include <QFontMetricsF>
#include <QJsonArray>
#include <QPainter>

namespace lumen::plot {

CalloutAnnotation::CalloutAnnotation(const QString& text, QPointF boxPosition,
                                      QPointF targetPoint, Anchor anchor)
    : text_(text)
    , boxPos_(boxPosition)
    , targetPt_(targetPoint)
{
    anchor_ = anchor;
}

void CalloutAnnotation::paint(QPainter* painter, const CoordinateMapper& mapper,
                               const QRectF& plotArea) const
{
    QPointF pBox = resolvePoint(boxPos_, anchor_, mapper, plotArea);
    QPointF pTarget = resolvePoint(targetPt_, anchor_, mapper, plotArea);

    // Measure text.
    QFontMetricsF fm(painter->font());
    QRectF textRect = fm.boundingRect(text_);
    textRect.moveTopLeft(pBox + QPointF(6, 4));
    QRectF boxRect = textRect.adjusted(-6, -4, 6, 4);

    // Draw arrow from box center to target.
    painter->setPen(QPen(Qt::darkGray, 1.0));
    painter->drawLine(boxRect.center(), pTarget);

    // Draw box.
    painter->setBrush(fillColor_);
    painter->setPen(QPen(Qt::darkGray, 0.5));
    painter->drawRoundedRect(boxRect, 4, 4);

    // Draw text.
    painter->setPen(textColor_);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text_);
}

QRectF CalloutAnnotation::boundingRect(const CoordinateMapper& mapper,
                                        const QRectF& plotArea) const
{
    QPointF pBox = resolvePoint(boxPos_, anchor_, mapper, plotArea);
    QPointF pTarget = resolvePoint(targetPt_, anchor_, mapper, plotArea);

    QFont defaultFont;
    QFontMetricsF fm(defaultFont);
    QRectF textRect = fm.boundingRect(text_);
    QRectF boxRect = textRect.adjusted(-6, -4, 6, 4);
    boxRect.moveTopLeft(pBox);

    return boxRect.united(QRectF(pTarget, QSizeF(1, 1))).adjusted(-5, -5, 5, 5);
}

QJsonObject CalloutAnnotation::toJson() const
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("callout");
    obj[QStringLiteral("anchor")] = static_cast<int>(anchor_);
    obj[QStringLiteral("text")] = text_;
    obj[QStringLiteral("boxPosition")] = QJsonArray{boxPos_.x(), boxPos_.y()};
    obj[QStringLiteral("targetPoint")] = QJsonArray{targetPt_.x(), targetPt_.y()};
    obj[QStringLiteral("fillColor")] = fillColor_.name(QColor::HexArgb);
    obj[QStringLiteral("textColor")] = textColor_.name();
    obj[QStringLiteral("visible")] = visible_;
    obj[QStringLiteral("name")] = name_;
    return obj;
}

std::unique_ptr<Annotation> CalloutAnnotation::fromJson(const QJsonObject& obj)
{
    QJsonArray boxArr = obj[QStringLiteral("boxPosition")].toArray();
    QJsonArray targetArr = obj[QStringLiteral("targetPoint")].toArray();
    auto anchor = static_cast<Anchor>(obj[QStringLiteral("anchor")].toInt());

    auto ann = std::make_unique<CalloutAnnotation>(
        obj[QStringLiteral("text")].toString(),
        QPointF(boxArr[0].toDouble(), boxArr[1].toDouble()),
        QPointF(targetArr[0].toDouble(), targetArr[1].toDouble()),
        anchor);

    ann->setFillColor(QColor(obj[QStringLiteral("fillColor")].toString()));
    ann->setTextColor(QColor(obj[QStringLiteral("textColor")].toString()));
    ann->setVisible(obj[QStringLiteral("visible")].toBool(true));
    ann->setName(obj[QStringLiteral("name")].toString());
    return ann;
}

}  // namespace lumen::plot
