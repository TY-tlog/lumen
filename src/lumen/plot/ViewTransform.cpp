#include "plot/ViewTransform.h"

namespace lumen::plot {

ViewTransform::ViewTransform(double xMin, double xMax, double yMin, double yMax)
    : baseXMin_(xMin)
    , baseXMax_(xMax)
    , baseYMin_(yMin)
    , baseYMax_(yMax)
    , viewXMin_(xMin)
    , viewXMax_(xMax)
    , viewYMin_(yMin)
    , viewYMax_(yMax)
{
}

void ViewTransform::setBaseRange(double xMin, double xMax, double yMin,
                                 double yMax)
{
    baseXMin_ = xMin;
    baseXMax_ = xMax;
    baseYMin_ = yMin;
    baseYMax_ = yMax;
    viewXMin_ = xMin;
    viewXMax_ = xMax;
    viewYMin_ = yMin;
    viewYMax_ = yMax;
}

void ViewTransform::pan(double dxData, double dyData)
{
    viewXMin_ += dxData;
    viewXMax_ += dxData;
    viewYMin_ += dyData;
    viewYMax_ += dyData;
}

void ViewTransform::zoom(double factor, double centerX, double centerY)
{
    zoomX(factor, centerX);
    zoomY(factor, centerY);
}

void ViewTransform::zoomX(double factor, double centerX)
{
    // Distance from center to edges, scaled by 1/factor.
    const double leftDist = (centerX - viewXMin_) / factor;
    const double rightDist = (viewXMax_ - centerX) / factor;
    viewXMin_ = centerX - leftDist;
    viewXMax_ = centerX + rightDist;
}

void ViewTransform::zoomY(double factor, double centerY)
{
    const double bottomDist = (centerY - viewYMin_) / factor;
    const double topDist = (viewYMax_ - centerY) / factor;
    viewYMin_ = centerY - bottomDist;
    viewYMax_ = centerY + topDist;
}

void ViewTransform::reset()
{
    viewXMin_ = baseXMin_;
    viewXMax_ = baseXMax_;
    viewYMin_ = baseYMin_;
    viewYMax_ = baseYMax_;
}

} // namespace lumen::plot
