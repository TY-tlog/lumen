#include "VolumeItem.h"
#include "Ray.h"
#include "ShaderProgram.h"

#include "data/Volume3D.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

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

    BoundingBox3D box = dataBounds();
    if (!box.isValid())
        return;

    QMatrix4x4 model;
    QMatrix4x4 mvp = ctx.vpMatrix * model;
    shader.setUniform(QStringLiteral("uMVP"), mvp);
    shader.setUniform(QStringLiteral("uModel"), model);
    QMatrix3x3 normalMat = model.normalMatrix();
    shader.setUniformMat3(QStringLiteral("uNormalMatrix"), normalMat);

    auto* f = QOpenGLContext::currentContext()->functions();

    if (gpuDirty_ || !vao_.isCreated()) {
        QColor tfColor = transferFunction_.sample(0.5);
        float r = tfColor.redF();
        float g = tfColor.greenF();
        float b = tfColor.blueF();

        float x0 = box.min.x(), y0 = box.min.y(), z0 = box.min.z();
        float x1 = box.max.x(), y1 = box.max.y(), z1 = box.max.z();

        // 24 vertices (4 per face, 6 faces) with outward normals.
        // clang-format off
        float verts[] = {
            // Front face (z1)
            x0,y0,z1, 0,0,1, r,g,b,  x1,y0,z1, 0,0,1, r,g,b,
            x1,y1,z1, 0,0,1, r,g,b,  x0,y1,z1, 0,0,1, r,g,b,
            // Back face (z0)
            x1,y0,z0, 0,0,-1, r,g,b,  x0,y0,z0, 0,0,-1, r,g,b,
            x0,y1,z0, 0,0,-1, r,g,b,  x1,y1,z0, 0,0,-1, r,g,b,
            // Top face (y1)
            x0,y1,z0, 0,1,0, r,g,b,  x1,y1,z0, 0,1,0, r,g,b,
            x1,y1,z1, 0,1,0, r,g,b,  x0,y1,z1, 0,1,0, r,g,b,
            // Bottom face (y0)
            x0,y0,z0, 0,-1,0, r,g,b,  x1,y0,z0, 0,-1,0, r,g,b,
            x1,y0,z1, 0,-1,0, r,g,b,  x0,y0,z1, 0,-1,0, r,g,b,
            // Right face (x1)
            x1,y0,z0, 1,0,0, r,g,b,  x1,y1,z0, 1,0,0, r,g,b,
            x1,y1,z1, 1,0,0, r,g,b,  x1,y0,z1, 1,0,0, r,g,b,
            // Left face (x0)
            x0,y0,z0, -1,0,0, r,g,b,  x0,y0,z1, -1,0,0, r,g,b,
            x0,y1,z1, -1,0,0, r,g,b,  x0,y1,z0, -1,0,0, r,g,b,
        };
        uint32_t idx[] = {
            0,1,2, 0,2,3,       // front
            4,5,6, 4,6,7,       // back
            8,9,10, 8,10,11,    // top
            12,13,14, 12,14,15, // bottom
            16,17,18, 16,18,19, // right
            20,21,22, 20,22,23, // left
        };
        // clang-format on

        if (!vao_.isCreated()) {
            vao_.create();
            vbo_.create();
            ebo_.create();
        }

        vao_.bind();
        vbo_.bind();
        vbo_.allocate(verts, static_cast<int>(sizeof(verts)));

        constexpr int kStride = 9 * static_cast<int>(sizeof(float));
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, kStride, nullptr);
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, kStride,
                                 reinterpret_cast<const void*>(3 * sizeof(float)));
        f->glEnableVertexAttribArray(2);
        f->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, kStride,
                                 reinterpret_cast<const void*>(6 * sizeof(float)));

        ebo_.bind();
        ebo_.allocate(idx, static_cast<int>(sizeof(idx)));

        vao_.release();
        vbo_.release();
        gpuDirty_ = false;
    }

    vao_.bind();
    f->glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    vao_.release();
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
