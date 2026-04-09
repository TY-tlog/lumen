#include "plot/LineSeries.h"

#include "data/Column.h"
#include "data/ColumnType.h"

#include <QPointF>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace lumen::plot {

LineSeries::LineSeries(const data::Column* xCol, const data::Column* yCol,
                       PlotStyle style, QString name)
    : xCol_(xCol)
    , yCol_(yCol)
    , style_(std::move(style))
    , name_(std::move(name))
{
    if (xCol_ == nullptr || yCol_ == nullptr) {
        throw std::invalid_argument("LineSeries: columns must not be null");
    }
    if (xCol_->type() != data::ColumnType::Double) {
        throw std::invalid_argument("LineSeries: X column must be Double type");
    }
    if (yCol_->type() != data::ColumnType::Double) {
        throw std::invalid_argument("LineSeries: Y column must be Double type");
    }
    if (xCol_->rowCount() != yCol_->rowCount()) {
        throw std::invalid_argument(
            "LineSeries: X and Y columns must have the same row count");
    }
}

std::vector<QPolygonF> LineSeries::buildPolylines() const
{
    std::vector<QPolygonF> result;

    const auto& xData = xCol_->doubleData();
    const auto& yData = yCol_->doubleData();
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
    const auto& xData = xCol_->doubleData();
    const auto& yData = yCol_->doubleData();
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
