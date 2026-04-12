#include "plot/ContourPlot.h"

#include "data/Grid2D.h"
#include "plot/ContourAlgorithm.h"
#include "plot/CoordinateMapper.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QString>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace lumen::plot {

ContourPlot::ContourPlot(std::shared_ptr<data::Grid2D> grid)
    : grid_(std::move(grid))
{
    if (!grid_) {
        throw std::invalid_argument("ContourPlot: grid must not be null");
    }
}

QRectF ContourPlot::dataBounds() const
{
    if (!grid_) {
        return {};
    }

    auto dims = grid_->dimensions();
    std::size_t cols = dims[0].length;
    std::size_t rows = dims[1].length;

    if (cols == 0 || rows == 0) {
        return {};
    }

    double xStart = dims[0].coordinates.valueAt(0);
    double xEnd = dims[0].coordinates.valueAt(cols - 1);
    double yStart = dims[1].coordinates.valueAt(0);
    double yEnd = dims[1].coordinates.valueAt(rows - 1);

    double xMin = std::min(xStart, xEnd);
    double xMax = std::max(xStart, xEnd);
    double yMin = std::min(yStart, yEnd);
    double yMax = std::max(yStart, yEnd);

    return QRectF(QPointF(xMin, yMin), QPointF(xMax, yMax));
}

void ContourPlot::computeAutoLevels()
{
    if (autoLevelCount_ <= 0 || !grid_) {
        return;
    }

    auto shape = grid_->shape();
    std::size_t cols = shape[0];
    std::size_t rows = shape[1];

    double vMin = std::numeric_limits<double>::max();
    double vMax = std::numeric_limits<double>::lowest();

    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < cols; ++c) {
            double v = grid_->valueAt({c, r});
            if (std::isnan(v)) {
                continue;
            }
            vMin = std::min(vMin, v);
            vMax = std::max(vMax, v);
        }
    }

    if (vMin >= vMax) {
        levels_.clear();
        return;
    }

    levels_.clear();
    levels_.reserve(static_cast<std::size_t>(autoLevelCount_));

    // Generate levels between vMin and vMax (exclusive of endpoints).
    double step = (vMax - vMin) / (autoLevelCount_ + 1);
    for (int i = 1; i <= autoLevelCount_; ++i) {
        levels_.push_back(vMin + i * step);
    }
}

void ContourPlot::paint(QPainter* painter, const CoordinateMapper& mapper,
                        const QRectF& plotArea) const
{
    if (!visible_ || !grid_) {
        return;
    }

    if (levels_.empty()) {
        return;
    }

    // Extract contour segments.
    auto segments = ContourAlgorithm::extract(*grid_, levels_);

    painter->save();
    painter->setClipRect(plotArea);

    QPen contourPen(lineColor_, lineWidth_);
    contourPen.setCosmetic(true);
    painter->setPen(contourPen);
    painter->setBrush(Qt::NoBrush);

    for (const auto& seg : segments) {
        QPointF pa = mapper.dataToPixel(seg.a.x(), seg.a.y());
        QPointF pb = mapper.dataToPixel(seg.b.x(), seg.b.y());
        painter->drawLine(pa, pb);
    }

    // Draw labels at segment midpoints if enabled.
    if (labelsVisible_ && !segments.empty()) {
        QFont font = painter->font();
        font.setPointSizeF(7.0);
        painter->setFont(font);
        painter->setPen(lineColor_);

        QFontMetricsF fm(font);

        // Draw one label per level, picking the first segment for that level.
        double lastLabelLevel = std::numeric_limits<double>::quiet_NaN();
        for (const auto& seg : segments) {
            if (seg.level == lastLabelLevel) {
                continue;
            }
            lastLabelLevel = seg.level;

            QPointF pa = mapper.dataToPixel(seg.a.x(), seg.a.y());
            QPointF pb = mapper.dataToPixel(seg.b.x(), seg.b.y());
            QPointF mid((pa.x() + pb.x()) / 2.0, (pa.y() + pb.y()) / 2.0);

            QString label = QString::number(seg.level, 'g', 4);
            QRectF textRect = fm.boundingRect(label);
            textRect.moveCenter(mid);

            painter->drawText(textRect, Qt::AlignCenter, label);
        }
    }

    painter->restore();
}

void ContourPlot::setAutoLevels(int count)
{
    autoLevelCount_ = std::max(count, 1);
    computeAutoLevels();
}

void ContourPlot::setLevels(std::vector<double> levels)
{
    autoLevelCount_ = 0;
    levels_ = std::move(levels);
}

void ContourPlot::setLabelsVisible(bool visible)
{
    labelsVisible_ = visible;
}

void ContourPlot::setLineColor(QColor color)
{
    lineColor_ = std::move(color);
}

void ContourPlot::setLineWidth(double width)
{
    lineWidth_ = std::max(width, 0.1);
}

void ContourPlot::setName(QString name)
{
    name_ = std::move(name);
}

void ContourPlot::setVisible(bool visible)
{
    visible_ = visible;
}

} // namespace lumen::plot
