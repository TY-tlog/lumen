#pragma once

#include <QVector3D>

namespace lumen::plot3d {

enum class LightType { Directional, Point, Ambient };

struct Light {
    LightType type = LightType::Directional;
    QVector3D position{0, 10, 0};
    QVector3D direction{0, -1, -1};
    QVector3D color{1, 1, 1};
    float intensity = 1.0f;
};

}  // namespace lumen::plot3d
