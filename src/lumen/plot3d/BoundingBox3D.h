#pragma once

#include <QVector3D>

#include <algorithm>
#include <limits>

namespace lumen::plot3d {

struct BoundingBox3D {
    QVector3D min{0, 0, 0};
    QVector3D max{0, 0, 0};

    [[nodiscard]] bool isValid() const
    {
        return min.x() <= max.x() && min.y() <= max.y() && min.z() <= max.z();
    }

    [[nodiscard]] BoundingBox3D united(const BoundingBox3D& other) const
    {
        if (!isValid())
            return other;
        if (!other.isValid())
            return *this;

        return BoundingBox3D{
            QVector3D(std::min(min.x(), other.min.x()),
                      std::min(min.y(), other.min.y()),
                      std::min(min.z(), other.min.z())),
            QVector3D(std::max(max.x(), other.max.x()),
                      std::max(max.y(), other.max.y()),
                      std::max(max.z(), other.max.z()))};
    }

    [[nodiscard]] QVector3D center() const
    {
        return (min + max) * 0.5f;
    }

    [[nodiscard]] QVector3D size() const
    {
        return max - min;
    }
};

}  // namespace lumen::plot3d
