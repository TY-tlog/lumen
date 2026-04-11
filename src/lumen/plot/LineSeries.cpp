#include "plot/LineSeries.h"

#include "data/Rank1Dataset.h"
#include "plot/CoordinateMapper.h"

#include <QPen>
#include <QPointF>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace lumen::plot {

LineSeries::LineSeries(std::shared_ptr<data::Rank1Dataset> xDs,
                       std::shared_ptr<data::Rank1Dataset> yDs,
                       PlotStyle style, QString name)
    : xDs_(std::move(xDs))
    , yDs_(std::move(yDs))
    , style_(std::move(style))
    , name_(std::move(name))
{
    if (xDs_ == nullptr || yDs_ == nullptr) {
        throw std::invalid_argument("LineSeries: datasets must not be null");
    }
    // Verify double data by attempting access (will throw bad_variant_access if wrong type)
    try {
        (void)xDs_->doubleData();
    } catch (...) {
        throw std::invalid_argument("LineSeries: X dataset must be Double type");
    }
    try {
        (void)yDs_->doubleData();
    } catch (...) {
        throw std::invalid_argument("LineSeries: Y dataset must be Double type");
    }
    if (xDs_->rowCount() != yDs_->rowCount()) {
        throw std::invalid_argument(
            "LineSeries: X and Y datasets must have the same row count");
    }
}

std::vector<QPolygonF> LineSeries::buildPolylines() const
{
    std::vector<QPolygonF> result;

    const auto& xData = xDs_->doubleData();
    const auto& yData = yDs_->doubleData();
    const auto count = xData.size();

    QPolygonF current;
    for (size_t i = 0; i < count; ++i) {
        const double x = xData[i];
        const double y = yData[i];

        if (std::isnan(x) || std::isnan(y)) {
            // NaN breaks the polyline. Save current segment if non-empty.
            if (!current.isEmpty()) {
                result.push_back(std::move(current));
                current = QPolygonF{};
            }
            continue;
        }

        current.append(QPointF{x, y});
    }

    // Don't forget the last segment.
    if (!current.isEmpty()) {
        result.push_back(std::move(current));
    }

    return result;
}

DataRange LineSeries::dataRange() const
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
        return DataRange{0.0, 0.0, 0.0, 0.0};
    }

    return DataRange{xMin, xMax, yMin, yMax};
}

QRectF LineSeries::dataBounds() const
{
    auto range = dataRange();
    return QRectF(QPointF(range.xMin, range.yMin),
                  QPointF(range.xMax, range.yMax));
}

void LineSeries::paint(QPainter* painter, const CoordinateMapper& mapper,
                       const QRectF& /*plotArea*/) const
{
    QPen seriesPen(style_.color, style_.lineWidth, style_.penStyle);
    seriesPen.setCosmetic(true);
    painter->setPen(seriesPen);
    painter->setBrush(Qt::NoBrush);

    auto polylines = buildPolylines();
    for (const auto& poly : polylines) {
        // Map data points to pixels.
        QPolygonF pixelPoly;
        pixelPoly.reserve(poly.size());
        for (const auto& pt : poly) {
            pixelPoly.append(mapper.dataToPixel(pt.x(), pt.y()));
        }
        painter->drawPolyline(pixelPoly);
    }
}

void LineSeries::setStyle(PlotStyle style) {
    style_ = std::move(style);
}

void LineSeries::setName(QString name) {
    name_ = std::move(name);
}

void LineSeries::setVisible(bool visible) {
    visible_ = visible;
}

} // namespace lumen::plot
