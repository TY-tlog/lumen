#include "TransferFunction.h"

#include <algorithm>
#include <cmath>

namespace lumen::plot3d {

void TransferFunction::addControlPoint(double value, QColor color, double opacity)
{
    ControlPoint cp;
    cp.value = value;
    cp.color = std::move(color);
    cp.opacity = opacity;
    points_.push_back(cp);
    std::sort(points_.begin(), points_.end());
}

void TransferFunction::setControlPoints(std::vector<ControlPoint> points)
{
    points_ = std::move(points);
    std::sort(points_.begin(), points_.end());
}

QColor TransferFunction::sample(double t) const
{
    if (points_.empty())
        return QColor(255, 255, 255, 255);

    t = std::clamp(t, 0.0, 1.0);

    if (points_.size() == 1 || t <= points_.front().value) {
        QColor c = points_.front().color;
        c.setAlphaF(static_cast<float>(points_.front().opacity));
        return c;
    }

    if (t >= points_.back().value) {
        QColor c = points_.back().color;
        c.setAlphaF(static_cast<float>(points_.back().opacity));
        return c;
    }

    // Find bracketing control points.
    for (std::size_t i = 0; i + 1 < points_.size(); ++i) {
        if (t >= points_[i].value && t <= points_[i + 1].value) {
            double range = points_[i + 1].value - points_[i].value;
            double frac = (range > 1e-15) ? (t - points_[i].value) / range : 0.0;

            const auto& a = points_[i];
            const auto& b = points_[i + 1];

            int r = static_cast<int>(std::round(a.color.red() * (1.0 - frac) + b.color.red() * frac));
            int g = static_cast<int>(std::round(a.color.green() * (1.0 - frac) + b.color.green() * frac));
            int bv = static_cast<int>(std::round(a.color.blue() * (1.0 - frac) + b.color.blue() * frac));
            double alpha = a.opacity * (1.0 - frac) + b.opacity * frac;

            QColor c(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(bv, 0, 255));
            c.setAlphaF(static_cast<float>(std::clamp(alpha, 0.0, 1.0)));
            return c;
        }
    }

    QColor c = points_.back().color;
    c.setAlphaF(static_cast<float>(points_.back().opacity));
    return c;
}

QImage TransferFunction::toLUT(int resolution) const
{
    QImage lut(resolution, 1, QImage::Format_RGBA8888);

    for (int i = 0; i < resolution; ++i) {
        double t = (resolution > 1) ? static_cast<double>(i) / (resolution - 1) : 0.0;
        QColor c = sample(t);
        lut.setPixelColor(i, 0, c);
    }

    return lut;
}

QJsonObject TransferFunction::toJson() const
{
    QJsonObject obj;
    QJsonArray arr;

    for (const auto& cp : points_) {
        QJsonObject cpObj;
        cpObj[QStringLiteral("value")] = cp.value;
        cpObj[QStringLiteral("r")] = cp.color.red();
        cpObj[QStringLiteral("g")] = cp.color.green();
        cpObj[QStringLiteral("b")] = cp.color.blue();
        cpObj[QStringLiteral("opacity")] = cp.opacity;
        arr.append(cpObj);
    }

    obj[QStringLiteral("controlPoints")] = arr;
    return obj;
}

TransferFunction TransferFunction::fromJson(const QJsonObject& obj)
{
    TransferFunction tf;
    QJsonArray arr = obj[QStringLiteral("controlPoints")].toArray();

    for (const auto& val : arr) {
        QJsonObject cpObj = val.toObject();
        ControlPoint cp;
        cp.value = cpObj[QStringLiteral("value")].toDouble();
        cp.color = QColor(cpObj[QStringLiteral("r")].toInt(),
                          cpObj[QStringLiteral("g")].toInt(),
                          cpObj[QStringLiteral("b")].toInt());
        cp.opacity = cpObj[QStringLiteral("opacity")].toDouble();
        tf.points_.push_back(cp);
    }

    std::sort(tf.points_.begin(), tf.points_.end());
    return tf;
}

}  // namespace lumen::plot3d
