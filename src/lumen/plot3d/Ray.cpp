#include "Ray.h"
#include "Camera.h"

namespace lumen::plot3d {

Ray Ray::fromScreenPixel(QPoint pixel, const Camera& cam, QSize viewport)
{
    // Convert screen pixel to normalized device coordinates (NDC).
    float x = (2.0f * static_cast<float>(pixel.x()) / static_cast<float>(viewport.width())) - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(pixel.y()) / static_cast<float>(viewport.height()));

    float aspect = static_cast<float>(viewport.width()) / static_cast<float>(viewport.height());
    QMatrix4x4 proj = cam.projectionMatrix(aspect);
    QMatrix4x4 view = cam.viewMatrix();

    // Invert view-projection to get world-space ray.
    QMatrix4x4 invVP = (proj * view).inverted();

    QVector4D nearPoint(x, y, -1.0f, 1.0f);
    QVector4D farPoint(x, y, 1.0f, 1.0f);

    QVector4D nearWorld = invVP * nearPoint;
    QVector4D farWorld = invVP * farPoint;

    if (std::abs(nearWorld.w()) > 1e-7f)
        nearWorld /= nearWorld.w();
    if (std::abs(farWorld.w()) > 1e-7f)
        farWorld /= farWorld.w();

    QVector3D origin(nearWorld.x(), nearWorld.y(), nearWorld.z());
    QVector3D dir = QVector3D(farWorld.x() - nearWorld.x(),
                               farWorld.y() - nearWorld.y(),
                               farWorld.z() - nearWorld.z())
                        .normalized();

    return Ray{origin, dir};
}

}  // namespace lumen::plot3d
