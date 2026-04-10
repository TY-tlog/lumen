#pragma once

#include <QColor>
#include <QPainter>
#include <QRectF>
#include <QString>

namespace lumen::plot {
class CoordinateMapper;

class PlotItem {
public:
    enum class Type { Line, Scatter, Bar };
    virtual ~PlotItem() = default;
    virtual Type type() const = 0;
    virtual QRectF dataBounds() const = 0;
    virtual void paint(QPainter* painter,
                       const CoordinateMapper& mapper,
                       const QRectF& plotArea) const = 0;
    virtual bool isVisible() const = 0;
    virtual QString name() const = 0;
    virtual QColor primaryColor() const = 0;
};
}  // namespace lumen::plot
