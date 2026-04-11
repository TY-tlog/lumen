#include "plot/ScatterSeries.h"

#include "data/Rank1Dataset.h"
#include "plot/CoordinateMapper.h"

#include <QPen>
#include <QPointF>
#include <QPolygonF>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace lumen::plot {

ScatterSeries::ScatterSeries(std::shared_ptr<data::Rank1Dataset> xDs,
                             std::shared_ptr<data::Rank1Dataset> yDs,
                             QColor color, QString name)
    : xDs_(std::move(xDs))
    , yDs_(std::move(yDs))
    , color_(std::move(color))
    , name_(std::move(name))
{
    if (xDs_ == nullptr || yDs_ == nullptr) {
        throw std::invalid_argument("ScatterSeries: datasets must not be null");
    }
    try {
        (void)xDs_->doubleData();
    } catch (...) {
        throw std::invalid_argument("ScatterSeries: X dataset must be Double type");
    }
    try {
        (void)yDs_->doubleData();
    } catch (...) {
        throw std::invalid_argument("ScatterSeries: Y dataset must be Double type");
    }
    if (xDs_->rowCount() != yDs_->rowCount()) {
        throw std::invalid_argument(
            "ScatterSeries: X and Y datasets must have the same row count");
    }
}

QRectF ScatterSeries::dataBounds() const
{
    const auto& xData = xDs_->doubleData();
    const auto& yData = yDs_->doubleData();
    const auto count = xData.size();

    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();

    bool hasValidPoint = false;

    for (size_t i = 0; i < count; ++i) {
        const double x = xData[i];
        const double y = yData[i];

        if (std::isnan(x) || std::isnan(y)) {
            continue;
        }

        hasValidPoint = true;
        xMin = std::min(xMin, x);
        xMax = std::max(xMax, x);
        yMin = std::min(yMin, y);
        yMax = std::max(yMax, y);
    }

    if (!hasValidPoint) {
        return QRectF(0.0, 0.0, 0.0, 0.0);
    }

    return QRectF(QPointF(xMin, yMin), QPointF(xMax, yMax));
}

void ScatterSeries::paint(QPainter* painter, const CoordinateMapper& mapper,
                          const QRectF& plotArea) const
{
    if (!visible_) {
        return;
    }

    painter->save();
    painter->setClipRect(plotArea);

    // Configure pen and brush based on filled_ state.
    if (filled_) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(color_);
    } else {
        QPen pen(color_, 1.5);
        pen.setCosmetic(true);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
    }

    const auto& xData = xDs_->doubleData();
    const auto& yData = yDs_->doubleData();
    const auto count = xData.size();

    for (size_t i = 0; i < count; ++i) {
        const double x = xData[i];
        const double y = yData[i];

        if (std::isnan(x) || std::isnan(y)) {
            continue;
        }

        QPointF pixel = mapper.dataToPixel(x, y);
        drawMarker(painter, pixel);
    }

    painter->restore();
}

void ScatterSeries::drawMarker(QPainter* painter, QPointF center) const
{
    const double r = markerSize_ / 2.0;

    switch (shape_) {
        case MarkerShape::Circle:
            painter->drawEllipse(center, r, r);
            break;

        case MarkerShape::Square:
            painter->drawRect(QRectF(center.x() - r, center.y() - r,
                                     markerSize_, markerSize_));
            break;

        case MarkerShape::Triangle: {
            QPolygonF tri;
            tri << QPointF(center.x(), center.y() - r)
                << QPointF(center.x() - r, center.y() + r)
                << QPointF(center.x() + r, center.y() + r);
            painter->drawPolygon(tri);
            break;
        }

        case MarkerShape::Diamond: {
            QPolygonF diamond;
            diamond << QPointF(center.x(), center.y() - r)
                    << QPointF(center.x() + r, center.y())
                    << QPointF(center.x(), center.y() + r)
                    << QPointF(center.x() - r, center.y());
            painter->drawPolygon(diamond);
            break;
        }

        case MarkerShape::Plus: {
            // For Plus and Cross, always use pen (never fill).
            QPen pen(color_, 1.5);
            pen.setCosmetic(true);
            painter->setPen(pen);
            painter->drawLine(QPointF(center.x() - r, center.y()),
                              QPointF(center.x() + r, center.y()));
            painter->drawLine(QPointF(center.x(), center.y() - r),
                              QPointF(center.x(), center.y() + r));
            // Restore pen/brush state for subsequent markers.
            if (filled_) {
                painter->setPen(Qt::NoPen);
            }
            break;
        }

        case MarkerShape::Cross: {
            QPen pen(color_, 1.5);
            pen.setCosmetic(true);
            painter->setPen(pen);
            painter->drawLine(QPointF(center.x() - r, center.y() - r),
                              QPointF(center.x() + r, center.y() + r));
            painter->drawLine(QPointF(center.x() + r, center.y() - r),
                              QPointF(center.x() - r, center.y() + r));
            // Restore pen/brush state for subsequent markers.
            if (filled_) {
                painter->setPen(Qt::NoPen);
            }
            break;
        }
    }
}

void ScatterSeries::setColor(QColor c) {
    color_ = std::move(c);
}

void ScatterSeries::setMarkerShape(MarkerShape shape) {
    shape_ = shape;
}

void ScatterSeries::setMarkerSize(int pixels) {
    markerSize_ = std::clamp(pixels, 3, 20);
}

void ScatterSeries::setFilled(bool filled) {
    filled_ = filled;
}

void ScatterSeries::setName(QString name) {
    name_ = std::move(name);
}

void ScatterSeries::setVisible(bool visible) {
    visible_ = visible;
}

} // namespace lumen::plot
