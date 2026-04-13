#include "VolumeItem.h"
#include "Ray.h"
#include "ShaderProgram.h"

#include "data/Volume3D.h"

#include <QMatrix4x4>

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumen::plot3d {

VolumeItem::VolumeItem(std::shared_ptr<data::Volume3D> volume, QString name)
    : volume_(std::move(volume))
    , name_(std::move(name))
{
    // Set up a default transfer function: black (transparent) to white (opaque).
    transferFunction_.addControlPoint(0.0, QColor(0, 0, 0), 0.0);
    transferFunction_.addControlPoint(1.0, QColor(255, 255, 255), 1.0);
}

VolumeItem::~VolumeItem() = default;

BoundingBox3D VolumeItem::dataBounds() const
{
    if (!volume_)
        return BoundingBox3D{};

    auto dims = volume_->dimensions();
    if (dims.size() < 3)
        return BoundingBox3D{};

    float xMin = static_cast<float>(dims[0].coordinates.valueAt(0));
    float xMax = static_cast<float>(dims[0].coordinates.valueAt(dims[0].length - 1));
    float yMin = static_cast<float>(dims[1].coordinates.valueAt(0));
    float yMax = static_cast<float>(dims[1].coordinates.valueAt(dims[1].length - 1));
    float zMin = static_cast<float>(dims[2].coordinates.valueAt(0));
    float zMax = static_cast<float>(dims[2].coordinates.valueAt(dims[2].length - 1));

    return BoundingBox3D{QVector3D(xMin, yMin, zMin),
                          QVector3D(xMax, yMax, zMax)};
}

void VolumeItem::render(ShaderProgram& shader, const RenderContext& ctx)
{
    if (!volume_)
        return;

    // Phase 8: proxy cube rendering with texture slicing.
    // Set up model matrix and uniforms.
    QMatrix4x4 model;
    QMatrix4x4 mvp = ctx.vpMatrix * model;
    shader.setUniform(QStringLiteral("uMVP"), mvp);
    shader.setUniform(QStringLiteral("uModel"), model);
    QMatrix3x3 normalMat = model.normalMatrix();
    shader.setUniformMat3(QStringLiteral("uNormalMatrix"), normalMat);

    // Generate the 1D transfer function LUT for upload.
    QImage lut = transferFunction_.toLUT(256);
    Q_UNUSED(lut);

    // Actual slice rendering deferred to GL context.
}

std::optional<HitResult3D> VolumeItem::hitTestRay(const Ray& ray,
                                                     double maxDist) const
{
    // Ray-AABB intersection for the volume bounding box.
    BoundingBox3D box = dataBounds();
    if (!box.isValid())
        return std::nullopt;

    float tMin = 0.0f;
    float tMax = static_cast<float>(maxDist);

    for (int i = 0; i < 3; ++i) {
        float bMin = (i == 0) ? box.min.x() : (i == 1) ? box.min.y() : box.min.z();
        float bMax = (i == 0) ? box.max.x() : (i == 1) ? box.max.y() : box.max.z();
        float orig = (i == 0) ? ray.origin.x() : (i == 1) ? ray.origin.y() : ray.origin.z();
        float dir = (i == 0) ? ray.direction.x() : (i == 1) ? ray.direction.y() : ray.direction.z();

        if (std::abs(dir) < 1e-8f) {
            if (orig < bMin || orig > bMax)
                return std::nullopt;
        } else {
            float invD = 1.0f / dir;
            float t0 = (bMin - orig) * invD;
            float t1 = (bMax - orig) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            tMin = std::max(tMin, t0);
            tMax = std::min(tMax, t1);
            if (tMax < tMin)
                return std::nullopt;
        }
    }

    HitResult3D hit;
    hit.itemIndex = -1;
    hit.distance = static_cast<double>(tMin);
    hit.worldPoint = ray.pointAt(static_cast<double>(tMin));
    return hit;
}

void VolumeItem::invalidate()
{
    // Nothing cached to invalidate currently.
}

void VolumeItem::setTransferFunction(TransferFunction tf)
{
    transferFunction_ = std::move(tf);
}

void VolumeItem::setSampleStep(float step)
{
    sampleStep_ = step;
}

void VolumeItem::setMaxSamples(int count)
{
    maxSamples_ = count;
}

void VolumeItem::setName(QString name)
{
    name_ = std::move(name);
}

void VolumeItem::setVisible(bool v)
{
    visible_ = v;
}

}  // namespace lumen::plot3d
