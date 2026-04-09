#include "plot/Legend.h"

namespace lumen::plot {

Legend::Legend(QObject* parent)
    : QObject(parent) {}

void Legend::setPosition(LegendPosition pos) {
    if (position_ != pos) {
        position_ = pos;
        emit changed();
    }
}

LegendPosition Legend::position() const {
    return position_;
}

void Legend::setVisible(bool visible) {
    if (visible_ != visible) {
        visible_ = visible;
        emit changed();
    }
}

bool Legend::isVisible() const {
    return visible_;
}

}  // namespace lumen::plot
