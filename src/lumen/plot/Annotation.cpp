#include "Annotation.h"

#include "plot/CoordinateMapper.h"
#include "plot/annotations/ArrowAnnotation.h"
#include "plot/annotations/BoxAnnotation.h"
#include "plot/annotations/CalloutAnnotation.h"
#include "plot/annotations/ColorBar.h"
#include "plot/annotations/ScaleBar.h"
#include "plot/annotations/TextAnnotation.h"

namespace lumen::plot {

QPointF Annotation::resolvePoint(QPointF point, Anchor anch,
                                  const CoordinateMapper& mapper,
                                  const QRectF& plotArea) const
{
    switch (anch) {
    case Anchor::Data:
        return mapper.dataToPixel(point.x(), point.y());
    case Anchor::Pixel:
        return point;
    case Anchor::AxisFraction:
        return QPointF(plotArea.left() + point.x() * plotArea.width(),
                       plotArea.top() + (1.0 - point.y()) * plotArea.height());
    }
    return point;
}

std::unique_ptr<Annotation> Annotation::fromJson(const QJsonObject& obj)
{
    QString typeStr = obj[QStringLiteral("type")].toString();

    if (typeStr == QStringLiteral("arrow"))
        return ArrowAnnotation::fromJson(obj);
    if (typeStr == QStringLiteral("box"))
        return BoxAnnotation::fromJson(obj);
    if (typeStr == QStringLiteral("callout"))
        return CalloutAnnotation::fromJson(obj);
    if (typeStr == QStringLiteral("text"))
        return TextAnnotation::fromJson(obj);
    if (typeStr == QStringLiteral("scale_bar"))
        return ScaleBar::fromJson(obj);
    if (typeStr == QStringLiteral("color_bar"))
        return ColorBar::fromJson(obj);

    return nullptr;
}

}  // namespace lumen::plot
