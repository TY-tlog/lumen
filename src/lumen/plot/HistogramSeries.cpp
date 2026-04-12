#include "plot/HistogramSeries.h"

#include "data/Rank1Dataset.h"
#include "plot/CoordinateMapper.h"

#include <QPen>
#include <QPointF>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace lumen::plot {

namespace {

/// Compute standard deviation of a sorted vector.
double computeStdDev(const std::vector<double>& sorted)
{
    if (sorted.size() < 2) {
        return 0.0;
    }
    double n = static_cast<double>(sorted.size());
    double mean = std::accumulate(sorted.begin(), sorted.end(), 0.0) / n;
    double sumSqDev = 0.0;
    for (double v : sorted) {
        double d = v - mean;
        sumSqDev += d * d;
    }
    return std::sqrt(sumSqDev / n);
}

/// Compute IQR (inter-quartile range) of a sorted vector.
double computeIQR(const std::vector<double>& sorted)
{
    if (sorted.size() < 4) {
        return 0.0;
    }
    auto n = sorted.size();
    double q1 = sorted[n / 4];
    double q3 = sorted[3 * n / 4];
    return q3 - q1;
}

/// Filter out NaN and sort.
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

HistogramSeries::HistogramSeries(std::shared_ptr<data::Rank1Dataset> data)
    : data_(std::move(data))
{
    if (!data_) {
        throw std::invalid_argument("HistogramSeries: data must not be null");
    }
    try {
        (void)data_->doubleData();
    } catch (...) {
        throw std::invalid_argument("HistogramSeries: data must be Double type");
    }
}

int HistogramSeries::computeAutoBinCount() const
{
    auto sorted = sortedValid(data_->doubleData());
    auto n = static_cast<double>(sorted.size());

    if (sorted.size() < 2) {
        return 1;
    }

    double range = sorted.back() - sorted.front();
    if (range <= 0.0) {
        return 1;
    }

    double binWidth = 0.0;
    int bins = 0;

    switch (binRule_) {
    case BinRule::Scott: {
        double sd = computeStdDev(sorted);
        if (sd <= 0.0) {
            return 1;
        }
        binWidth = 3.49 * sd * std::pow(n, -1.0 / 3.0);
        bins = std::max(1, static_cast<int>(std::ceil(range / binWidth)));
        break;
    }
    case BinRule::FreedmanDiaconis: {
        double iqr = computeIQR(sorted);
        if (iqr <= 0.0) {
            return 1;
        }
        binWidth = 2.0 * iqr * std::pow(n, -1.0 / 3.0);
        bins = std::max(1, static_cast<int>(std::ceil(range / binWidth)));
        break;
    }
    case BinRule::Sturges:
        bins = static_cast<int>(std::ceil(std::log2(n))) + 1;
        break;
    }

    return std::max(bins, 1);
}

std::vector<HistogramSeries::Bin> HistogramSeries::computeBins() const
{
    auto sorted = sortedValid(data_->doubleData());

    if (sorted.empty()) {
        return {};
    }

    int nBins = (binCount_ > 0) ? binCount_ : computeAutoBinCount();

    double lo = sorted.front();
    double hi = sorted.back();

    if (lo == hi) {
        // All values identical: single bin.
        Bin b;
        b.lo = lo - 0.5;
        b.hi = lo + 0.5;
        b.value = static_cast<double>(sorted.size());
        if (normalization_ == Normalization::Probability) {
            b.value = 1.0;
        } else if (normalization_ == Normalization::Density) {
            b.value = static_cast<double>(sorted.size()) / (1.0 * static_cast<double>(sorted.size()));
        }
        return {b};
    }

    double binWidth = (hi - lo) / nBins;

    // Build bins with counts.
    std::vector<Bin> bins(static_cast<std::size_t>(nBins));
    for (int i = 0; i < nBins; ++i) {
        bins[static_cast<std::size_t>(i)].lo = lo + i * binWidth;
        bins[static_cast<std::size_t>(i)].hi = lo + (i + 1) * binWidth;
        bins[static_cast<std::size_t>(i)].value = 0.0;
    }

    // Count values into bins.
    for (double v : sorted) {
        int idx = static_cast<int>((v - lo) / binWidth);
        if (idx >= nBins) {
            idx = nBins - 1; // Include right edge in last bin.
        }
        if (idx < 0) {
            idx = 0;
        }
        bins[static_cast<std::size_t>(idx)].value += 1.0;
    }

    // Apply normalization.
    double n = static_cast<double>(sorted.size());
    switch (normalization_) {
    case Normalization::Count:
        // Already counts.
        break;
    case Normalization::Probability:
        for (auto& b : bins) {
            b.value /= n;
        }
        break;
    case Normalization::Density:
        for (auto& b : bins) {
            b.value /= (n * binWidth);
        }
        break;
    }

    return bins;
}

QRectF HistogramSeries::dataBounds() const
{
    auto bins = computeBins();
    if (bins.empty()) {
        return {};
    }

    double xMin = bins.front().lo;
    double xMax = bins.back().hi;
    double yMax = 0.0;
    for (const auto& b : bins) {
        yMax = std::max(yMax, b.value);
    }

    return QRectF(QPointF(xMin, 0.0), QPointF(xMax, yMax));
}

void HistogramSeries::paint(QPainter* painter, const CoordinateMapper& mapper,
                            const QRectF& plotArea) const
{
    if (!visible_) {
        return;
    }

    auto bins = computeBins();
    if (bins.empty()) {
        return;
    }

    painter->save();
    painter->setClipRect(plotArea);

    painter->setBrush(fillColor_);
    if (outlineColor_.alpha() > 0) {
        QPen pen(outlineColor_, 1);
        pen.setCosmetic(true);
        painter->setPen(pen);
    } else {
        painter->setPen(Qt::NoPen);
    }

    for (const auto& bin : bins) {
        QPointF topLeft = mapper.dataToPixel(bin.lo, bin.value);
        QPointF bottomRight = mapper.dataToPixel(bin.hi, 0.0);
        QRectF barRect(topLeft, bottomRight);
        painter->drawRect(barRect.normalized());
    }

    painter->restore();
}

void HistogramSeries::setBinCount(int n)
{
    binCount_ = std::max(n, 1);
}

void HistogramSeries::setAutoBinning(BinRule rule)
{
    binCount_ = 0;
    binRule_ = rule;
}

void HistogramSeries::setNormalization(Normalization n)
{
    normalization_ = n;
}

void HistogramSeries::setFillColor(QColor color)
{
    fillColor_ = std::move(color);
}

void HistogramSeries::setOutlineColor(QColor color)
{
    outlineColor_ = std::move(color);
}

void HistogramSeries::setName(QString name)
{
    name_ = std::move(name);
}

void HistogramSeries::setVisible(bool visible)
{
    visible_ = visible;
}

} // namespace lumen::plot
