#include "plot/PlotScene.h"

#include <algorithm>

namespace lumen::plot {

namespace {
constexpr double kLeftMargin = 60.0;
constexpr double kBottomMargin = 50.0;
constexpr double kTopMarginWithTitle = 30.0;
constexpr double kTopMarginNoTitle = 15.0;
constexpr double kRightMargin = 15.0;
}  // namespace

PlotScene::PlotScene() = default;

void PlotScene::addSeries(LineSeries series) {
    series_.push_back(std::move(series));
}

void PlotScene::clearSeries() {
    series_.clear();
}

void PlotScene::setTitle(const QString& title) {
    title_ = title;
}

QRectF PlotScene::computePlotArea(QSizeF widgetSize) const {
    double topMargin = title_.isEmpty() ? kTopMarginNoTitle : kTopMarginWithTitle;

    double x = kLeftMargin;
    double y = topMargin;
    double w = widgetSize.width() - kLeftMargin - kRightMargin;
    double h = widgetSize.height() - topMargin - kBottomMargin;

    w = std::max(w, 1.0);
    h = std::max(h, 1.0);

    return {x, y, w, h};
}

void PlotScene::autoRange() {
    xAxis_.autoRange(series_);
    yAxis_.autoRange(series_);
    viewTransform_.setBaseRange(xAxis_.min(), xAxis_.max(),
                                yAxis_.min(), yAxis_.max());
}

}  // namespace lumen::plot
