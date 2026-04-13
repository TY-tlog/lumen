#include "ScaleBar.h"

#include "plot/CoordinateMapper.h"

#include <QFontMetricsF>
#include <QJsonArray>
#include <QPainter>

namespace lumen::plot {

ScaleBar::ScaleBar(double lengthInDataUnits, const QString& unitLabel,
                   QPointF position, Anchor anchor)
    : lengthData_(lengthInDataUnits)
    , unitLabel_(unitLabel)
    , position_(position)
{
    anchor_ = anchor;
}

void ScaleBar::paint(QPainter* painter, const CoordinateMapper& mapper,
                      const QRectF& plotArea) const
{
    QPointF pos = resolvePoint(position_, anchor_, mapper, plotArea);

    // Compute pixel length: map (0, 0) and (lengthData_, 0) to pixels.
    QPointF origin = mapper.dataToPixel(0, 0);
    QPointF end = mapper.dataToPixel(lengthData_, 0);
    double pixelLength = std::abs(end.x() - origin.x());

    // Draw bar.
    painter->setPen(Qt::NoPen);
    painter->setBrush(color_);
    painter->drawRect(QRectF(pos.x(), pos.y(), pixelLength, barHeight_));

    // Draw label below.
    QString label = QStringLiteral("%1 %2").arg(lengthData_).arg(unitLabel_);
    painter->setPen(color_);
    QFont font = painter->font();
    font.setPixelSize(10);
    painter->setFont(font);
    QFontMetricsF fm(font);
    double textWidth = fm.horizontalAdvance(label);
    painter->drawText(
        QPointF(pos.x() + (pixelLength - textWidth) / 2.0,
                pos.y() + barHeight_ + fm.ascent() + 2),
        label);
}

QRectF ScaleBar::boundingRect(const CoordinateMapper& mapper,
                               const QRectF& plotArea) const
{
    QPointF pos = resolvePoint(position_, anchor_, mapper, plotArea);
    QPointF origin = mapper.dataToPixel(0, 0);
    QPointF end = mapper.dataToPixel(lengthData_, 0);
    double pixelLength = std::abs(end.x() - origin.x());
    return QRectF(pos.x() - 2, pos.y() - 2,
                  pixelLength + 4, barHeight_ + 20);
}

QJsonObject ScaleBar::toJson() const
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("scale_bar");
    obj[QStringLiteral("anchor")] = static_cast<int>(anchor_);
    obj[QStringLiteral("lengthData")] = lengthData_;
    obj[QStringLiteral("unitLabel")] = unitLabel_;
    obj[QStringLiteral("position")] = QJsonArray{position_.x(), position_.y()};
    obj[QStringLiteral("color")] = color_.name();
    obj[QStringLiteral("barHeight")] = barHeight_;
    obj[QStringLiteral("visible")] = visible_;
    obj[QStringLiteral("name")] = name_;
    return obj;
}

std::unique_ptr<Annotation> ScaleBar::fromJson(const QJsonObject& obj)
{
    QJsonArray posArr = obj[QStringLiteral("position")].toArray();
    auto anchor = static_cast<Anchor>(obj[QStringLiteral("anchor")].toInt());

    auto ann = std::make_unique<ScaleBar>(
        obj[QStringLiteral("lengthData")].toDouble(100),
        obj[QStringLiteral("unitLabel")].toString(),
        QPointF(posArr[0].toDouble(), posArr[1].toDouble()),
        anchor);

    ann->setColor(QColor(obj[QStringLiteral("color")].toString()));
    ann->setBarHeight(obj[QStringLiteral("barHeight")].toDouble(4.0));
    ann->setVisible(obj[QStringLiteral("visible")].toBool(true));
    ann->setName(obj[QStringLiteral("name")].toString());
    return ann;
}

}  // namespace lumen::plot
