#include "GridLayout.h"

#include <algorithm>

namespace lumen::dashboard {

void GridLayout::setGridSize(int rows, int cols)
{
    rows_ = std::clamp(rows, 1, kMaxRows);
    cols_ = std::clamp(cols, 1, kMaxCols);
}

void GridLayout::setSpacing(double px)
{
    spacing_ = std::max(0.0, px);
}

QRectF GridLayout::cellRect(const PanelConfig& panel,
                             QSizeF availableSize) const
{
    double totalSpacingX = spacing_ * static_cast<double>(cols_ - 1);
    double totalSpacingY = spacing_ * static_cast<double>(rows_ - 1);

    double cellW = (availableSize.width() - totalSpacingX) /
                   static_cast<double>(cols_);
    double cellH = (availableSize.height() - totalSpacingY) /
                   static_cast<double>(rows_);

    if (cellW < 1.0) cellW = 1.0;
    if (cellH < 1.0) cellH = 1.0;

    int c = std::clamp(panel.col, 0, cols_ - 1);
    int r = std::clamp(panel.row, 0, rows_ - 1);
    int cs = std::clamp(panel.colSpan, 1, cols_ - c);
    int rs = std::clamp(panel.rowSpan, 1, rows_ - r);

    double x = static_cast<double>(c) * (cellW + spacing_);
    double y = static_cast<double>(r) * (cellH + spacing_);
    double w = static_cast<double>(cs) * cellW +
               static_cast<double>(cs - 1) * spacing_;
    double h = static_cast<double>(rs) * cellH +
               static_cast<double>(rs - 1) * spacing_;

    return QRectF(x, y, w, h);
}

std::vector<QRectF> GridLayout::allRects(
    const std::vector<PanelConfig>& panels, QSizeF availableSize) const
{
    std::vector<QRectF> rects;
    rects.reserve(panels.size());
    for (const auto& p : panels) {
        rects.push_back(cellRect(p, availableSize));
    }
    return rects;
}

}  // namespace lumen::dashboard
