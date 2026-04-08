#include "plot/CoordinateMapper.h"

namespace lumen::plot {

CoordinateMapper::CoordinateMapper(double xMin, double xMax, double yMin,
                                   double yMax, QRectF pixelRect)
    : xMin_(xMin)
    , xMax_(xMax)
    , yMin_(yMin)
    , yMax_(yMax)
    , pixelRect_(pixelRect)
{
}

CoordinateMapper::CoordinateMapper() = default;

QPointF CoordinateMapper::dataToPixel(double x, double y) const
{
    const double xRange = xMax_ - xMin_;
    const double yRange = yMax_ - yMin_;

    // Linear interpolation: data -> [0, 1] -> pixel.
    const double xNorm = (xRange != 0.0) ? (x - xMin_) / xRange : 0.5;
    // Y inverted: higher data Y -> lower pixel Y.
    const double yNorm = (yRange != 0.0) ? (yMax_ - y) / yRange : 0.5;

    const double px = pixelRect_.left() + xNorm * pixelRect_.width();
    const double py = pixelRect_.top() + yNorm * pixelRect_.height();

    return {px, py};
}

std::pair<double, double> CoordinateMapper::pixelToData(QPointF pixel) const
{
    const double xNorm = (pixelRect_.width() != 0.0)
                             ? (pixel.x() - pixelRect_.left()) / pixelRect_.width()
                             : 0.5;
    const double yNorm = (pixelRect_.height() != 0.0)
                             ? (pixel.y() - pixelRect_.top()) / pixelRect_.height()
                             : 0.5;

    const double x = xMin_ + xNorm * (xMax_ - xMin_);
    // Y inverted: lower pixel Y -> higher data Y.
    const double y = yMax_ - yNorm * (yMax_ - yMin_);

    return {x, y};
}

void CoordinateMapper::setDataRange(double xMin, double xMax, double yMin,
                                    double yMax)
{
    xMin_ = xMin;
    xMax_ = xMax;
    yMin_ = yMin;
    yMax_ = yMax;
}

void CoordinateMapper::setPixelRect(QRectF rect)
{
    pixelRect_ = rect;
}

} // namespace lumen::plot
