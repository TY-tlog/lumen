#pragma once

#include <QPoint>
#include <QSize>
#include <QVector3D>

namespace lumen::plot3d {

class Camera;

struct Ray {
    QVector3D origin;
    QVector3D direction;  // normalized

    static Ray fromScreenPixel(QPoint pixel, const Camera& cam, QSize viewport);

    [[nodiscard]] QVector3D pointAt(double t) const
    {
        return origin + direction * static_cast<float>(t);
    }
};

}  // namespace lumen::plot3d
