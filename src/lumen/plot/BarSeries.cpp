#include "plot/BarSeries.h"

#include "data/Column.h"
#include "data/ColumnType.h"
#include "plot/CoordinateMapper.h"

#include <QPen>
#include <QPointF>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace lumen::plot {

BarSeries::BarSeries(const data::Column* xCol, const data::Column* yCol,
                     QColor fillColor, QString name)
    : xCol_(xCol)
    , yCol_(yCol)
    , fillColor_(std::move(fillColor))
    , name_(std::move(name))
{
    if (xCol_ == nullptr || yCol_ == nullptr) {
        throw std::invalid_argument("BarSeries: columns must not be null");
    }
    if (xCol_->type() != data::ColumnType::Double) {
        throw std::invalid_argument("BarSeries: X column must be Double type");
    }
    if (yCol_->type() != data::ColumnType::Double) {
        throw std::invalid_argument("BarSeries: Y column must be Double type");
    }
    if (xCol_->rowCount() != yCol_->rowCount()) {
        throw std::invalid_argument(
            "BarSeries: X and Y columns must have the same row count");
    }
}

QRectF BarSeries::dataBounds() const
{
    const auto& xData = xCol_->doubleData();
    const auto& yData = yCol_->doubleData();
    const auto count = xData.size();

    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMin = 0.0;  // Always include baseline y=0.
    double yMax = 0.0;

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

double BarSeries::computeMedianXSpacing() const
{
    if (cachedMedianSpacing_ >= 0.0) {
        return cachedMedianSpacing_;
    }

    const auto& xData = xCol_->doubleData();

    // Collect valid (non-NaN) X values and sort them.
    std::vector<double> validX;
    validX.reserve(xData.size());
    for (double x : xData) {
        if (!std::isnan(x)) {
            validX.push_back(x);
        }
    }

    if (validX.size() < 2) {
        // Single point or empty: use fallback of 1.0.
        cachedMedianSpacing_ = 1.0;
        return cachedMedianSpacing_;
    }

    std::sort(validX.begin(), validX.end());

    // Compute consecutive differences.
    std::vector<double> diffs;
    diffs.reserve(validX.size() - 1);
    for (size_t i = 1; i < validX.size(); ++i) {
        double d = validX[i] - validX[i - 1];
        if (d > 0.0) {
            diffs.push_back(d);
        }
    }

    if (diffs.empty()) {
        cachedMedianSpacing_ = 1.0;
        return cachedMedianSpacing_;
    }

    std::sort(diffs.begin(), diffs.end());
    cachedMedianSpacing_ = diffs[diffs.size() / 2];
    return cachedMedianSpacing_;
}

void BarSeries::paint(QPainter* painter, const CoordinateMapper& mapper,
                      const QRectF& plotArea) const
{
    if (!visible_) {
        return;
    }

    painter->save();
    painter->setClipRect(plotArea);

    const auto& xData = xCol_->doubleData();
    const auto& yData = yCol_->doubleData();
    const auto count = xData.size();

    const double medianSpacing = computeMedianXSpacing();
    const double halfWidth = (barWidth_ * medianSpacing) / 2.0;

    // Minimum bar width of 2px.
    constexpr double kMinBarWidthPx = 2.0;

    painter->setBrush(fillColor_);

    bool hasOutline = outlineColor_.alpha() > 0;
    if (hasOutline) {
        QPen outlinePen(outlineColor_, 1);
        outlinePen.setCosmetic(true);
        painter->setPen(outlinePen);
    } else {
        painter->setPen(Qt::NoPen);
    }

    for (size_t i = 0; i < count; ++i) {
        const double x = xData[i];
        const double y = yData[i];

        if (std::isnan(x) || std::isnan(y)) {
            continue;
        }

        // Compute bar rect in data space: [x - halfWidth, 0] to [x + halfWidth, y].
        QPointF topLeft = mapper.dataToPixel(x - halfWidth, std::max(y, 0.0));
        QPointF bottomRight = mapper.dataToPixel(x + halfWidth, std::min(y, 0.0));

        // Ensure minimum bar width in pixels.
        double pixelWidth = std::abs(bottomRight.x() - topLeft.x());
        if (pixelWidth < kMinBarWidthPx) {
            double center = (topLeft.x() + bottomRight.x()) / 2.0;
            topLeft.setX(center - kMinBarWidthPx / 2.0);
            bottomRight.setX(center + kMinBarWidthPx / 2.0);
        }

        QRectF barRect(topLeft, bottomRight);
        painter->drawRect(barRect.normalized());
    }

    painter->restore();
}

void BarSeries::setFillColor(QColor c) {
    fillColor_ = std::move(c);
    // Don't reset cached spacing -- it depends only on X data, not color.
}

void BarSeries::setOutlineColor(QColor c) {
    outlineColor_ = std::move(c);
}

void BarSeries::setBarWidth(double relative) {
    barWidth_ = std::clamp(relative, 0.1, 1.0);
}

void BarSeries::setName(QString name) {
    name_ = std::move(name);
}

void BarSeries::setVisible(bool visible) {
    visible_ = visible;
}

} // namespace lumen::plot
