#include "Isosurface.h"
#include "Ray.h"
#include "ShaderProgram.h"

#include "data/Volume3D.h"

#include <QMatrix4x4>

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumen::plot3d {

Isosurface::Isosurface(std::shared_ptr<data::Volume3D> volume, QString name)
    : volume_(std::move(volume))
    , name_(std::move(name))
{
}

Isosurface::~Isosurface() = default;

BoundingBox3D Isosurface::dataBounds() const
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

void Isosurface::render(ShaderProgram& shader, const RenderContext& ctx)
{
    const auto& ms = meshes();
    if (ms.empty())
        return;

    QMatrix4x4 model;
    QMatrix4x4 mvp = ctx.vpMatrix * model;
    shader.setUniform(QStringLiteral("uMVP"), mvp);
    shader.setUniform(QStringLiteral("uModel"), model);
    QMatrix3x3 normalMat = model.normalMatrix();
    shader.setUniformMat3(QStringLiteral("uNormalMatrix"), normalMat);

    for (std::size_t mi = 0; mi < ms.size(); ++mi) {
        const auto& m = ms[mi];
        if (m.empty())
            continue;

        // Determine color for this iso surface.
        QColor color = (mi < colors_.size()) ? colors_[mi] : QColor(100, 149, 237);
        float r = color.redF();
        float g = color.greenF();
        float b = color.blueF();

        // Build interleaved vertex data.
        std::vector<float> vertexData;
        vertexData.reserve(m.vertices.size() * 9);
        for (const auto& v : m.vertices) {
            vertexData.push_back(v.position.x());
            vertexData.push_back(v.position.y());
            vertexData.push_back(v.position.z());
            vertexData.push_back(v.normal.x());
            vertexData.push_back(v.normal.y());
            vertexData.push_back(v.normal.z());
            vertexData.push_back(r);
            vertexData.push_back(g);
            vertexData.push_back(b);
        }

        Q_UNUSED(vertexData);
    }
}

std::optional<HitResult3D> Isosurface::hitTestRay(const Ray& ray,
                                                      double maxDist) const
{
    const auto& ms = meshes();
    float bestDist = static_cast<float>(maxDist);
    std::optional<HitResult3D> bestHit;

    for (const auto& m : ms) {
        // Moeller-Trumbore ray-triangle intersection.
        for (std::size_t i = 0; i + 2 < m.indices.size(); i += 3) {
            const QVector3D& v0 = m.vertices[m.indices[i]].position;
            const QVector3D& v1 = m.vertices[m.indices[i + 1]].position;
            const QVector3D& v2 = m.vertices[m.indices[i + 2]].position;

            QVector3D edge1 = v1 - v0;
            QVector3D edge2 = v2 - v0;
            QVector3D h = QVector3D::crossProduct(ray.direction, edge2);
            float a = QVector3D::dotProduct(edge1, h);

            if (std::abs(a) < 1e-7f)
                continue;

            float f = 1.0f / a;
            QVector3D s = ray.origin - v0;
            float u = f * QVector3D::dotProduct(s, h);
            if (u < 0.0f || u > 1.0f)
                continue;

            QVector3D q = QVector3D::crossProduct(s, edge1);
            float v = f * QVector3D::dotProduct(ray.direction, q);
            if (v < 0.0f || u + v > 1.0f)
                continue;

            float t = f * QVector3D::dotProduct(edge2, q);
            if (t < 0.0f || t >= bestDist)
                continue;

            bestDist = t;
            HitResult3D hit;
            hit.itemIndex = -1;
            hit.distance = static_cast<double>(t);
            hit.worldPoint = ray.pointAt(static_cast<double>(t));
            bestHit = hit;
        }
    }

    return bestHit;
}

void Isosurface::invalidate()
{
    meshesDirty_ = true;
}

void Isosurface::setIsoValues(std::vector<double> values)
{
    isoValues_ = std::move(values);
    meshesDirty_ = true;
}

void Isosurface::setColors(std::vector<QColor> colors)
{
    colors_ = std::move(colors);
}

void Isosurface::setName(QString name)
{
    name_ = std::move(name);
}

void Isosurface::setVisible(bool v)
{
    visible_ = v;
}

const std::vector<MarchingCubes::Mesh>& Isosurface::meshes() const
{
    if (meshesDirty_)
        buildMeshes();
    return meshes_;
}

std::size_t Isosurface::totalTriangleCount() const
{
    const auto& ms = meshes();
    std::size_t total = 0;
    for (const auto& m : ms)
        total += m.triangleCount();
    return total;
}

void Isosurface::buildMeshes() const
{
    meshes_.clear();
    meshesDirty_ = false;

    if (!volume_ || isoValues_.empty())
        return;

    for (double iso : isoValues_) {
        meshes_.push_back(MarchingCubes::extract(*volume_, iso));
    }
}

}  // namespace lumen::plot3d
