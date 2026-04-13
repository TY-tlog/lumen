#include "ColorBar.h"

#include "plot/CoordinateMapper.h"

#include <QFontMetricsF>
#include <QJsonArray>
#include <QPainter>

namespace lumen::plot {

ColorBar::ColorBar(Colormap colormap, double valueMin, double valueMax,
                   QPointF position, Anchor anchor)
    : colormap_(std::move(colormap))
    , valueMin_(valueMin)
    , valueMax_(valueMax)
    , position_(position)
{
    anchor_ = anchor;
}

void ColorBar::paint(QPainter* painter, const CoordinateMapper& mapper,
                      const QRectF& plotArea) const
{
    QPointF pos = resolvePoint(position_, anchor_, mapper, plotArea);

    bool vertical = (orientation_ == Orientation::Vertical);
    int steps = static_cast<int>(barLength_);

    for (int i = 0; i < steps; ++i) {
        double t = static_cast<double>(i) / static_cast<double>(steps - 1);
        QColor c = colormap_.sample(t);

        if (vertical) {
            // Draw from bottom to top.
            double y = pos.y() + barLength_ - static_cast<double>(i) - 1.0;
            painter->fillRect(QRectF(pos.x(), y, barWidth_, 1.0), c);
        } else {
            double x = pos.x() + static_cast<double>(i);
            painter->fillRect(QRectF(x, pos.y(), 1.0, barWidth_), c);
        }
    }

    // Border.
    painter->setPen(QPen(Qt::black, 0.5));
    painter->setBrush(Qt::NoBrush);
    if (vertical) {
        painter->drawRect(QRectF(pos.x(), pos.y(), barWidth_, barLength_));
    } else {
        painter->drawRect(QRectF(pos.x(), pos.y(), barLength_, barWidth_));
    }

    // Tick labels.
    QFont font = painter->font();
    font.setPixelSize(9);
    painter->setFont(font);
    painter->setPen(Qt::black);

    constexpr int kNumTicks = 5;
    for (int i = 0; i < kNumTicks; ++i) {
        double t = static_cast<double>(i) / static_cast<double>(kNumTicks - 1);
        double val = valueMin_ + t * (valueMax_ - valueMin_);
        QString label = QString::number(val, 'g', 3);

        if (vertical) {
            double y = pos.y() + barLength_ * (1.0 - t);
            painter->drawText(QPointF(pos.x() + barWidth_ + 4, y + 3), label);
        } else {
            double x = pos.x() + barLength_ * t;
            painter->drawText(QPointF(x - 8, pos.y() + barWidth_ + 12), label);
        }
    }

    // Unit label.
    if (!unitLabel_.isEmpty()) {
        if (vertical) {
            painter->drawText(
                QPointF(pos.x(), pos.y() - 5), unitLabel_);
        } else {
            painter->drawText(
                QPointF(pos.x() + barLength_ + 5, pos.y() + barWidth_ / 2.0 + 4),
                unitLabel_);
        }
    }
}

QRectF ColorBar::boundingRect(const CoordinateMapper& mapper,
                               const QRectF& plotArea) const
{
    QPointF pos = resolvePoint(position_, anchor_, mapper, plotArea);
    if (orientation_ == Orientation::Vertical) {
        return QRectF(pos.x() - 2, pos.y() - 15,
                      barWidth_ + 60, barLength_ + 20);
    }
    return QRectF(pos.x() - 2, pos.y() - 2,
                  barLength_ + 40, barWidth_ + 25);
}

QJsonObject ColorBar::toJson() const
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("color_bar");
    obj[QStringLiteral("anchor")] = static_cast<int>(anchor_);
    obj[QStringLiteral("position")] = QJsonArray{position_.x(), position_.y()};
    obj[QStringLiteral("colormap")] = colormap_.toJson();
    obj[QStringLiteral("valueMin")] = valueMin_;
    obj[QStringLiteral("valueMax")] = valueMax_;
    obj[QStringLiteral("orientation")] = (orientation_ == Orientation::Vertical)
                                             ? QStringLiteral("vertical")
                                             : QStringLiteral("horizontal");
    obj[QStringLiteral("unitLabel")] = unitLabel_;
    obj[QStringLiteral("visible")] = visible_;
    obj[QStringLiteral("name")] = name_;
    return obj;
}

std::unique_ptr<Annotation> ColorBar::fromJson(const QJsonObject& obj)
{
    QJsonArray posArr = obj[QStringLiteral("position")].toArray();
    auto anchor = static_cast<Anchor>(obj[QStringLiteral("anchor")].toInt());

    Colormap cmap = Colormap::builtin(Colormap::Builtin::Viridis);
    if (obj.contains(QStringLiteral("colormap")))
        cmap = Colormap::fromJson(obj[QStringLiteral("colormap")].toObject());

    auto ann = std::make_unique<ColorBar>(
        cmap,
        obj[QStringLiteral("valueMin")].toDouble(0.0),
        obj[QStringLiteral("valueMax")].toDouble(1.0),
        QPointF(posArr[0].toDouble(), posArr[1].toDouble()),
        anchor);

    if (obj[QStringLiteral("orientation")].toString() == QStringLiteral("horizontal"))
        ann->setOrientation(Orientation::Horizontal);
    ann->setUnitLabel(obj[QStringLiteral("unitLabel")].toString());
    ann->setVisible(obj[QStringLiteral("visible")].toBool(true));
    ann->setName(obj[QStringLiteral("name")].toString());
    return ann;
}

}  // namespace lumen::plot
