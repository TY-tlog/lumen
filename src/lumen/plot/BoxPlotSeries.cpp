#include "plot/BoxPlotSeries.h"

#include "data/Rank1Dataset.h"
#include "plot/CoordinateMapper.h"

#include <QPen>
#include <QPointF>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace lumen::plot {

namespace {

/// Compute the percentile of a sorted vector using linear interpolation.
double percentile(const std::vector<double>& sorted, double p)
{
    if (sorted.empty()) {
        return 0.0;
    }
    if (sorted.size() == 1) {
        return sorted[0];
    }

    double rank = p / 100.0 * static_cast<double>(sorted.size() - 1);
    auto lo = static_cast<std::size_t>(std::floor(rank));
    auto hi = static_cast<std::size_t>(std::ceil(rank));
    if (lo == hi) {
        return sorted[lo];
    }
    double frac = rank - static_cast<double>(lo);
    return sorted[lo] * (1.0 - frac) + sorted[hi] * frac;
}

/// Filter NaN and sort.
std::vector<double> sortedValid(const std::vector<double>& raw)
{
    std::vector<double> result;
    result.reserve(raw.size());
    for (double v : raw) {
        if (!std::isnan(v)) {
            result.push_back(v);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

} // anonymous namespace

BoxPlotSeries::BoxPlotSeries(std::shared_ptr<data::Rank1Dataset> data)
    : data_(std::move(data))
{
    if (!data_) {
        throw std::invalid_argument("BoxPlotSeries: data must not be null");
    }
    try {
        (void)data_->doubleData();
    } catch (...) {
        throw std::invalid_argument("BoxPlotSeries: data must be Double type");
    }
}

BoxPlotSeries::Stats BoxPlotSeries::computeStats() const
{
    auto sorted = sortedValid(data_->doubleData());

    Stats stats{};

    if (sorted.empty()) {
        return stats;
    }

    stats.median = percentile(sorted, 50.0);
    stats.q1 = percentile(sorted, 25.0);
    stats.q3 = percentile(sorted, 75.0);

    double iqr = stats.q3 - stats.q1;

    switch (whiskerRule_) {
    case WhiskerRule::Tukey: {
        double lowerFence = stats.q1 - 1.5 * iqr;
        double upperFence = stats.q3 + 1.5 * iqr;

        // Whiskers extend to the most extreme data point within the fences.
        stats.whiskerLo = sorted.front();
        stats.whiskerHi = sorted.back();
        for (double v : sorted) {
            if (v >= lowerFence) {
                stats.whiskerLo = v;
                break;
            }
        }
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
            if (*it <= upperFence) {
                stats.whiskerHi = *it;
                break;
            }
        }

        // Outliers are values beyond the fences.
        for (double v : sorted) {
            if (v < lowerFence || v > upperFence) {
                stats.outliers.push_back(v);
            }
        }
        break;
    }
    case WhiskerRule::MinMax:
        stats.whiskerLo = sorted.front();
        stats.whiskerHi = sorted.back();
        // No outliers in MinMax mode.
        break;
    case WhiskerRule::Percentile:
        stats.whiskerLo = percentile(sorted, 5.0);
        stats.whiskerHi = percentile(sorted, 95.0);
        // Outliers are values beyond the 5th/95th percentile.
        for (double v : sorted) {
            if (v < stats.whiskerLo || v > stats.whiskerHi) {
                stats.outliers.push_back(v);
            }
        }
        break;
    }

    return stats;
}

QRectF BoxPlotSeries::dataBounds() const
{
    auto stats = computeStats();
    auto sorted = sortedValid(data_->doubleData());

    if (sorted.empty()) {
        return {};
    }

    double yMin = sorted.front();
    double yMax = sorted.back();

    double halfW = boxWidth_ / 2.0;
    return QRectF(QPointF(position_ - halfW, yMin),
                  QPointF(position_ + halfW, yMax));
}

void BoxPlotSeries::paint(QPainter* painter, const CoordinateMapper& mapper,
                          const QRectF& plotArea) const
{
    if (!visible_) {
        return;
    }

    auto sorted = sortedValid(data_->doubleData());
    if (sorted.empty()) {
        return;
    }

    auto stats = computeStats();

    painter->save();
    painter->setClipRect(plotArea);

    double halfW = boxWidth_ / 2.0;
    double xLeft = position_ - halfW;
    double xRight = position_ + halfW;

    // Draw the box (Q1 to Q3).
    QPointF boxTopLeft = mapper.dataToPixel(xLeft, stats.q3);
    QPointF boxBottomRight = mapper.dataToPixel(xRight, stats.q1);
    QRectF boxRect(boxTopLeft, boxBottomRight);

    painter->setBrush(fillColor_);
    QPen outlinePen(outlineColor_, 1.5);
    outlinePen.setCosmetic(true);
    painter->setPen(outlinePen);

    if (notched_) {
        // Notched box: narrow at the median.
        double n = static_cast<double>(sorted.size());
        double notchHalf = 1.58 * (stats.q3 - stats.q1) / std::sqrt(n);
        double notchLo = stats.median - notchHalf;
        double notchHi = stats.median + notchHalf;
        double notchIndent = boxWidth_ * 0.15;

        QPolygonF notchPoly;
        notchPoly << mapper.dataToPixel(xLeft, stats.q3);
        notchPoly << mapper.dataToPixel(xRight, stats.q3);
        notchPoly << mapper.dataToPixel(xRight, notchHi);
        notchPoly << mapper.dataToPixel(position_ + halfW - notchIndent, stats.median);
        notchPoly << mapper.dataToPixel(xRight, notchLo);
        notchPoly << mapper.dataToPixel(xRight, stats.q1);
        notchPoly << mapper.dataToPixel(xLeft, stats.q1);
        notchPoly << mapper.dataToPixel(xLeft, notchLo);
        notchPoly << mapper.dataToPixel(position_ - halfW + notchIndent, stats.median);
        notchPoly << mapper.dataToPixel(xLeft, notchHi);

        painter->drawPolygon(notchPoly);
    } else {
        painter->drawRect(boxRect.normalized());
    }

    // Draw median line.
    QPen medianPen(outlineColor_, 2.0);
    medianPen.setCosmetic(true);
    painter->setPen(medianPen);
    QPointF medianLeft = mapper.dataToPixel(xLeft, stats.median);
    QPointF medianRight = mapper.dataToPixel(xRight, stats.median);
    painter->drawLine(medianLeft, medianRight);

    // Draw whiskers.
    QPen whiskerPen(outlineColor_, 1.0);
    whiskerPen.setCosmetic(true);
    painter->setPen(whiskerPen);

    // Lower whisker: vertical line from Q1 to whiskerLo.
    QPointF wLoBot = mapper.dataToPixel(position_, stats.whiskerLo);
    QPointF wLoTop = mapper.dataToPixel(position_, stats.q1);
    painter->drawLine(wLoBot, wLoTop);

    // Lower whisker cap.
    double capHalf = halfW * 0.5;
    QPointF capLoLeft = mapper.dataToPixel(position_ - capHalf, stats.whiskerLo);
    QPointF capLoRight = mapper.dataToPixel(position_ + capHalf, stats.whiskerLo);
    painter->drawLine(capLoLeft, capLoRight);

    // Upper whisker: vertical line from Q3 to whiskerHi.
    QPointF wHiBot = mapper.dataToPixel(position_, stats.q3);
    QPointF wHiTop = mapper.dataToPixel(position_, stats.whiskerHi);
    painter->drawLine(wHiBot, wHiTop);

    // Upper whisker cap.
    QPointF capHiLeft = mapper.dataToPixel(position_ - capHalf, stats.whiskerHi);
    QPointF capHiRight = mapper.dataToPixel(position_ + capHalf, stats.whiskerHi);
    painter->drawLine(capHiLeft, capHiRight);

    // Draw outliers.
    if (outliersVisible_ && !stats.outliers.empty()) {
        painter->setBrush(Qt::NoBrush);
        QPen outlierPen(outlineColor_, 1.0);
        outlierPen.setCosmetic(true);
        painter->setPen(outlierPen);

        constexpr double kOutlierRadius = 3.0;
        for (double v : stats.outliers) {
            QPointF center = mapper.dataToPixel(position_, v);
            painter->drawEllipse(center, kOutlierRadius, kOutlierRadius);
        }
    }

    painter->restore();
}

void BoxPlotSeries::setWhiskerRule(WhiskerRule rule)
{
    whiskerRule_ = rule;
}

void BoxPlotSeries::setNotched(bool notched)
{
    notched_ = notched;
}

void BoxPlotSeries::setOutliersVisible(bool visible)
{
    outliersVisible_ = visible;
}

void BoxPlotSeries::setFillColor(QColor color)
{
    fillColor_ = std::move(color);
}

void BoxPlotSeries::setOutlineColor(QColor color)
{
    outlineColor_ = std::move(color);
}

void BoxPlotSeries::setPosition(double x)
{
    position_ = x;
}

void BoxPlotSeries::setBoxWidth(double w)
{
    boxWidth_ = std::max(w, 0.01);
}

void BoxPlotSeries::setName(QString name)
{
    name_ = std::move(name);
}

void BoxPlotSeries::setVisible(bool visible)
{
    visible_ = visible;
}

} // namespace lumen::plot
