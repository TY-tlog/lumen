#pragma once

#include "PanelConfig.h"

#include <QRectF>
#include <QSizeF>

#include <vector>

namespace lumen::dashboard {

class GridLayout {
public:
    void setGridSize(int rows, int cols);
    [[nodiscard]] int rows() const { return rows_; }
    [[nodiscard]] int cols() const { return cols_; }

    void setSpacing(double px);
    [[nodiscard]] double spacing() const { return spacing_; }

    [[nodiscard]] QRectF cellRect(const PanelConfig& panel,
                                   QSizeF availableSize) const;

    [[nodiscard]] std::vector<QRectF> allRects(
        const std::vector<PanelConfig>& panels,
        QSizeF availableSize) const;

    static constexpr int kMaxRows = 8;
    static constexpr int kMaxCols = 8;

private:
    int rows_ = 1;
    int cols_ = 1;
    double spacing_ = 8.0;
};

}  // namespace lumen::dashboard
