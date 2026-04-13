#include "ArrowAnnotation.h"

#include "plot/CoordinateMapper.h"

#include <QJsonArray>
#include <QPainter>

#include <cmath>

namespace lumen::plot {

ArrowAnnotation::ArrowAnnotation(QPointF from, QPointF to, Anchor anchor)
    : from_(from)
    , to_(to)
{
    anchor_ = anchor;
}

void ArrowAnnotation::paint(QPainter* painter, const CoordinateMapper& mapper,
                             const QRectF& plotArea) const
{
    QPointF pFrom = resolvePoint(from_, anchor_, mapper, plotArea);
    QPointF pTo = resolvePoint(to_, anchor_, mapper, plotArea);

    QPen pen(color_, lineWidth_);
    painter->setPen(pen);
    painter->drawLine(pFrom, pTo);

    // Draw arrowhead at 'to' end.
    double dx = pTo.x() - pFrom.x();
    double dy = pTo.y() - pFrom.y();
    double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-6)
        return;

    double headLen = std::min(12.0, len * 0.3);
    double headAngle = 0.4;  // ~23 degrees
    double angle = std::atan2(dy, dx);

    QPointF left(pTo.x() - headLen * std::cos(angle - headAngle),
                 pTo.y() - headLen * std::sin(angle - headAngle));
    QPointF right(pTo.x() - headLen * std::cos(angle + headAngle),
                  pTo.y() - headLen * std::sin(angle + headAngle));

    if (headStyle_ == HeadStyle::Filled) {
        painter->setBrush(color_);
        QPolygonF head;
        head << pTo << left << right;
        painter->drawPolygon(head);
    } else {
        painter->drawLine(pTo, left);
        painter->drawLine(pTo, right);
    }

    if (headStyle_ == HeadStyle::Double) {
        // Also draw head at 'from' end.
        QPointF l2(pFrom.x() + headLen * std::cos(angle - headAngle),
                   pFrom.y() + headLen * std::sin(angle - headAngle));
        QPointF r2(pFrom.x() + headLen * std::cos(angle + headAngle),
                   pFrom.y() + headLen * std::sin(angle + headAngle));
        painter->setBrush(color_);
        QPolygonF head2;
        head2 << pFrom << l2 << r2;
        painter->drawPolygon(head2);
    }
}

QRectF ArrowAnnotation::boundingRect(const CoordinateMapper& mapper,
                                      const QRectF& plotArea) const
{
    QPointF pFrom = resolvePoint(from_, anchor_, mapper, plotArea);
    QPointF pTo = resolvePoint(to_, anchor_, mapper, plotArea);
    return QRectF(pFrom, pTo).normalized().adjusted(-5, -5, 5, 5);
}

QJsonObject ArrowAnnotation::toJson() const
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("arrow");
    obj[QStringLiteral("anchor")] = static_cast<int>(anchor_);
    obj[QStringLiteral("from")] = QJsonArray{from_.x(), from_.y()};
    obj[QStringLiteral("to")] = QJsonArray{to_.x(), to_.y()};
    obj[QStringLiteral("color")] = color_.name();
    obj[QStringLiteral("lineWidth")] = lineWidth_;
    obj[QStringLiteral("headStyle")] = static_cast<int>(headStyle_);
    obj[QStringLiteral("visible")] = visible_;
    obj[QStringLiteral("name")] = name_;
    return obj;
}

std::unique_ptr<Annotation> ArrowAnnotation::fromJson(const QJsonObject& obj)
{
    QJsonArray fromArr = obj[QStringLiteral("from")].toArray();
    QJsonArray toArr = obj[QStringLiteral("to")].toArray();
    auto anchor = static_cast<Anchor>(obj[QStringLiteral("anchor")].toInt());

    auto ann = std::make_unique<ArrowAnnotation>(
        QPointF(fromArr[0].toDouble(), fromArr[1].toDouble()),
        QPointF(toArr[0].toDouble(), toArr[1].toDouble()),
        anchor);

    ann->setColor(QColor(obj[QStringLiteral("color")].toString()));
    ann->setLineWidth(obj[QStringLiteral("lineWidth")].toDouble(1.5));
    ann->setHeadStyle(static_cast<HeadStyle>(obj[QStringLiteral("headStyle")].toInt()));
    ann->setVisible(obj[QStringLiteral("visible")].toBool(true));
    ann->setName(obj[QStringLiteral("name")].toString());
    return ann;
}

}  // namespace lumen::plot
