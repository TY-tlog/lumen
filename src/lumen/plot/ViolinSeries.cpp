#include "plot/ViolinSeries.h"

#include "data/Rank1Dataset.h"
#include "plot/CoordinateMapper.h"

#include <QPen>
#include <QPointF>
#include <QPolygonF>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace lumen::plot {

namespace {

constexpr int kKdePoints = 100;
constexpr double kPi = 3.14159265358979323846;

/// Gaussian kernel: (1/sqrt(2*pi)) * exp(-0.5 * x^2)
double gaussianKernel(double x)
{
    return (1.0 / std::sqrt(2.0 * kPi)) * std::exp(-0.5 * x * x);
}

/// Compute standard deviation of a vector.
double computeStdDev(const std::vector<double>& values)
{
    if (values.size() < 2) {
        return 0.0;
    }
    double n = static_cast<double>(values.size());
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / n;
    double sumSqDev = 0.0;
    for (double v : values) {
        double d = v - mean;
        sumSqDev += d * d;
    }
    return std::sqrt(sumSqDev / n);
}

/// Compute IQR from sorted data.
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

ViolinSeries::ViolinSeries(std::shared_ptr<data::Rank1Dataset> data)
    : data_(std::move(data))
{
    if (!data_) {
        throw std::invalid_argument("ViolinSeries: data must not be null");
    }
    try {
        (void)data_->doubleData();
    } catch (...) {
        throw std::invalid_argument("ViolinSeries: data must be Double type");
    }
}

double ViolinSeries::computeSilvermanBandwidth() const
{
    auto sorted = sortedValid(data_->doubleData());
    if (sorted.size() < 2) {
        return 1.0;
    }

    double sd = computeStdDev(sorted);
    double iqr = computeIQR(sorted);
    double n = static_cast<double>(sorted.size());

    double spread = sd;
    if (iqr > 0.0) {
        spread = std::min(sd, iqr / 1.34);
    }

    if (spread <= 0.0) {
        return 1.0;
    }

    // Silverman: 0.9 * min(std, IQR/1.34) * n^(-1/5)
    return 0.9 * spread * std::pow(n, -0.2);
}

std::vector<ViolinSeries::KdePoint> ViolinSeries::computeKde() const
{
    auto sorted = sortedValid(data_->doubleData());

    if (sorted.empty()) {
        return {};
    }

    double h = autoBandwidth_ ? computeSilvermanBandwidth() : bandwidth_;
    if (h <= 0.0) {
        h = 1.0;
    }

    double yMin = sorted.front();
    double yMax = sorted.back();

    if (yMin == yMax) {
        // All values identical: single spike.
        return {{yMin, 1.0}};
    }

    // Extend range slightly beyond data.
    double extend = 3.0 * h;
    yMin -= extend;
    yMax += extend;

    double step = (yMax - yMin) / (kKdePoints - 1);

    std::vector<KdePoint> points;
    points.reserve(kKdePoints);

    double n = static_cast<double>(sorted.size());

    for (int i = 0; i < kKdePoints; ++i) {
        double y = yMin + i * step;
        double density = 0.0;
        for (double xi : sorted) {
            density += gaussianKernel((y - xi) / h);
        }
        density /= (n * h);
        points.push_back({y, density});
    }

    return points;
}

QRectF ViolinSeries::dataBounds() const
{
    auto sorted = sortedValid(data_->doubleData());
    if (sorted.empty()) {
        return {};
    }

    double yMin = sorted.front();
    double yMax = sorted.back();
    double halfW = maxWidth_;

    return QRectF(QPointF(position_ - halfW, yMin),
                  QPointF(position_ + halfW, yMax));
}

void ViolinSeries::paint(QPainter* painter, const CoordinateMapper& mapper,
                         const QRectF& plotArea) const
{
    if (!visible_) {
        return;
    }

    auto kdePoints = computeKde();
    if (kdePoints.empty()) {
        return;
    }

    // Find max density for scaling.
    double maxDensity = 0.0;
    for (const auto& pt : kdePoints) {
        maxDensity = std::max(maxDensity, pt.density);
    }
    if (maxDensity <= 0.0) {
        return;
    }

    painter->save();
    painter->setClipRect(plotArea);

    // Build the polygon. For a full violin, it's mirrored: right side then
    // left side in reverse. For split, just one side.
    QPolygonF poly;

    if (split_) {
        // Right side only.
        for (const auto& pt : kdePoints) {
            double w = (pt.density / maxDensity) * maxWidth_;
            poly.append(mapper.dataToPixel(position_ + w, pt.y));
        }
        // Close along the center line (bottom to top).
        for (auto it = kdePoints.rbegin(); it != kdePoints.rend(); ++it) {
            poly.append(mapper.dataToPixel(position_, it->y));
        }
    } else {
        // Right side (bottom to top).
        for (const auto& pt : kdePoints) {
            double w = (pt.density / maxDensity) * maxWidth_;
            poly.append(mapper.dataToPixel(position_ + w, pt.y));
        }
        // Left side (top to bottom, mirrored).
        for (auto it = kdePoints.rbegin(); it != kdePoints.rend(); ++it) {
            double w = (it->density / maxDensity) * maxWidth_;
            poly.append(mapper.dataToPixel(position_ - w, it->y));
        }
    }

    painter->setBrush(fillColor_);
    QPen outlinePen(outlineColor_, 1.0);
    outlinePen.setCosmetic(true);
    painter->setPen(outlinePen);

    painter->drawPolygon(poly);

    painter->restore();
}

void ViolinSeries::setKdeBandwidth(double h)
{
    bandwidth_ = std::max(h, 1e-6);
    autoBandwidth_ = false;
}

void ViolinSeries::setKdeBandwidthAuto(bool automatic)
{
    autoBandwidth_ = automatic;
}

void ViolinSeries::setSplit(bool split)
{
    split_ = split;
}

void ViolinSeries::setFillColor(QColor color)
{
    fillColor_ = std::move(color);
}

void ViolinSeries::setOutlineColor(QColor color)
{
    outlineColor_ = std::move(color);
}

void ViolinSeries::setPosition(double x)
{
    position_ = x;
}

void ViolinSeries::setMaxWidth(double w)
{
    maxWidth_ = std::max(w, 0.01);
}

void ViolinSeries::setName(QString name)
{
    name_ = std::move(name);
}

void ViolinSeries::setVisible(bool visible)
{
    visible_ = visible;
}

} // namespace lumen::plot
