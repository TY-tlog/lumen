#pragma once

#include "Light.h"

#include <QMatrix4x4>
#include <QSize>
#include <QVector3D>

#include <vector>

namespace lumen::plot3d {

struct RenderContext {
    QMatrix4x4 viewMatrix;
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 vpMatrix;  // view * projection
    QSize viewport;
    std::vector<Light> lights;
    QVector3D cameraPosition;
};

struct HitResult3D {
    int itemIndex = -1;
    double distance = 0.0;
    QVector3D worldPoint;
};

}  // namespace lumen::plot3d
