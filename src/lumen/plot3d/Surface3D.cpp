#include "Surface3D.h"
#include "Ray.h"
#include "ShaderProgram.h"

#include "data/Grid2D.h"
#include "plot/Colormap.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumen::plot3d {

Surface3D::Surface3D(std::shared_ptr<data::Grid2D> grid,
                     QColor baseColor, QString name)
    : grid_(std::move(grid))
    , baseColor_(std::move(baseColor))
    , name_(std::move(name))
{
}

Surface3D::~Surface3D() = default;

BoundingBox3D Surface3D::dataBounds() const
{
    if (!grid_)
        return BoundingBox3D{};

    auto dims = grid_->dimensions();
    if (dims.size() < 2)
        return BoundingBox3D{};

    const auto& dimX = dims[0];
    const auto& dimY = dims[1];

    float xMin = static_cast<float>(dimX.coordinates.valueAt(0));
    float xMax = static_cast<float>(dimX.coordinates.valueAt(dimX.length - 1));
    float yMin = static_cast<float>(dimY.coordinates.valueAt(0));
    float yMax = static_cast<float>(dimY.coordinates.valueAt(dimY.length - 1));

    // Find value range for height.
    const auto& data = grid_->data();
    double vMin = std::numeric_limits<double>::max();
    double vMax = std::numeric_limits<double>::lowest();
    for (double v : data) {
        vMin = std::min(vMin, v);
        vMax = std::max(vMax, v);
    }

    float zMin = (mode_ == Mode::FlatColored) ? 0.0f : static_cast<float>(vMin);
    float zMax = (mode_ == Mode::FlatColored) ? 0.0f : static_cast<float>(vMax);

    return BoundingBox3D{QVector3D(xMin, yMin, zMin),
                          QVector3D(xMax, yMax, zMax)};
}

void Surface3D::render(ShaderProgram& shader, const RenderContext& ctx)
{
    const auto& verts = vertices();
    const auto& idxs = indices();
    if (verts.empty() || idxs.empty())
        return;

    BoundingBox3D bounds = dataBounds();
    QVector3D sz = bounds.size();
    float xyMax = std::max(sz.x(), sz.y());
    float zRange = std::max(sz.z(), 0.001f);
    float zScale = (xyMax > 0.0f && zRange < xyMax * 0.1f)
                       ? xyMax * 0.3f / zRange
                       : 1.0f;

    QMatrix4x4 model;
    model.translate(0, 0, -bounds.center().z() * zScale);
    model.scale(1.0f, 1.0f, zScale);

    QMatrix4x4 mvp = ctx.vpMatrix * model;
    shader.setUniform(QStringLiteral("uMVP"), mvp);
    shader.setUniform(QStringLiteral("uModel"), model);
    QMatrix3x3 normalMat = model.normalMatrix();
    shader.setUniformMat3(QStringLiteral("uNormalMatrix"), normalMat);

    auto* f = QOpenGLContext::currentContext()->functions();

    if (gpuDirty_ || !vao_.isCreated()) {
        std::vector<float> vertexData;
        vertexData.reserve(verts.size() * 9);
        for (const auto& v : verts) {
            vertexData.push_back(v.position.x());
            vertexData.push_back(v.position.y());
            vertexData.push_back(v.position.z());
            vertexData.push_back(v.normal.x());
            vertexData.push_back(v.normal.y());
            vertexData.push_back(v.normal.z());
            vertexData.push_back(v.color.x());
            vertexData.push_back(v.color.y());
            vertexData.push_back(v.color.z());
        }

        if (!vao_.isCreated()) {
            vao_.create();
            vbo_.create();
            ebo_.create();
        }

        vao_.bind();

        vbo_.bind();
        vbo_.allocate(vertexData.data(),
                      static_cast<int>(vertexData.size() * sizeof(float)));

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
        ebo_.allocate(idxs.data(),
                      static_cast<int>(idxs.size() * sizeof(uint32_t)));

        vao_.release();
        vbo_.release();

        gpuIndexCount_ = static_cast<int>(idxs.size());
        gpuDirty_ = false;
    }

    vao_.bind();
    f->glDrawElements(GL_TRIANGLES, gpuIndexCount_, GL_UNSIGNED_INT, nullptr);
    vao_.release();
}

std::optional<HitResult3D> Surface3D::hitTestRay(const Ray& ray,
                                                   double maxDist) const
{
    const auto& verts = vertices();
    const auto& idxs = indices();

    float bestDist = static_cast<float>(maxDist);
    std::optional<HitResult3D> bestHit;

    // Sequential ray-triangle intersection using Moeller-Trumbore.
    for (std::size_t i = 0; i + 2 < idxs.size(); i += 3) {
        const QVector3D& v0 = verts[idxs[i]].position;
        const QVector3D& v1 = verts[idxs[i + 1]].position;
        const QVector3D& v2 = verts[idxs[i + 2]].position;

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

    return bestHit;
}

void Surface3D::invalidate()
{
    meshDirty_ = true;
    gpuDirty_ = true;
}

void Surface3D::setMode(Mode mode)
{
    if (mode_ != mode) {
        mode_ = mode;
        meshDirty_ = true;
    }
}

void Surface3D::setWireframe(bool enabled)
{
    wireframe_ = enabled;
}

void Surface3D::setColormap(std::shared_ptr<plot::Colormap> cmap)
{
    colormap_ = std::move(cmap);
    meshDirty_ = true;
}

void Surface3D::setName(QString name)
{
    name_ = std::move(name);
}

void Surface3D::setVisible(bool v)
{
    visible_ = v;
}

void Surface3D::setBaseColor(QColor c)
{
    baseColor_ = std::move(c);
    meshDirty_ = true;
}

const std::vector<Surface3D::Vertex>& Surface3D::vertices() const
{
    if (meshDirty_)
        buildMesh();
    return vertices_;
}

const std::vector<uint32_t>& Surface3D::indices() const
{
    if (meshDirty_)
        buildMesh();
    return indices_;
}

std::size_t Surface3D::triangleCount() const
{
    if (meshDirty_)
        buildMesh();
    return indices_.size() / 3;
}

void Surface3D::buildMesh() const
{
    vertices_.clear();
    indices_.clear();
    meshDirty_ = false;

    if (!grid_)
        return;

    std::size_t nx = grid_->cols();
    std::size_t ny = grid_->rows();

    if (nx < 2 || ny < 2)
        return;

    auto dims = grid_->dimensions();
    const auto& dimX = dims[0];
    const auto& dimY = dims[1];
    const auto& data = grid_->data();

    // Find value range for colormap normalization.
    double vMin = std::numeric_limits<double>::max();
    double vMax = std::numeric_limits<double>::lowest();
    for (double v : data) {
        vMin = std::min(vMin, v);
        vMax = std::max(vMax, v);
    }
    double vRange = vMax - vMin;
    if (vRange < 1e-15)
        vRange = 1.0;

    // Build vertex positions.
    vertices_.resize(nx * ny);
    for (std::size_t iy = 0; iy < ny; ++iy) {
        for (std::size_t ix = 0; ix < nx; ++ix) {
            float x = static_cast<float>(dimX.coordinates.valueAt(ix));
            float y = static_cast<float>(dimY.coordinates.valueAt(iy));
            double val = data[iy * nx + ix];

            float z = 0.0f;
            if (mode_ == Mode::HeightMap || mode_ == Mode::Both)
                z = static_cast<float>(val);

            std::size_t idx = iy * nx + ix;
            vertices_[idx].position = QVector3D(x, y, z);

            // Color from colormap or base color.
            double t = (val - vMin) / vRange;
            if (colormap_) {
                QColor c = colormap_->sample(t);
                vertices_[idx].color = QVector3D(
                    c.redF(),
                    c.greenF(),
                    c.blueF());
            } else {
                vertices_[idx].color = QVector3D(
                    baseColor_.redF(),
                    baseColor_.greenF(),
                    baseColor_.blueF());
            }

            // Normal initialized to zero; will accumulate face normals.
            vertices_[idx].normal = QVector3D(0, 0, 0);
        }
    }

    // Build triangle indices: 2*(nx-1)*(ny-1) triangles.
    std::size_t numTriangles = 2 * (nx - 1) * (ny - 1);
    indices_.reserve(numTriangles * 3);

    for (std::size_t iy = 0; iy < ny - 1; ++iy) {
        for (std::size_t ix = 0; ix < nx - 1; ++ix) {
            auto i00 = static_cast<uint32_t>(iy * nx + ix);
            auto i10 = static_cast<uint32_t>(iy * nx + ix + 1);
            auto i01 = static_cast<uint32_t>((iy + 1) * nx + ix);
            auto i11 = static_cast<uint32_t>((iy + 1) * nx + ix + 1);

            // Triangle 1: (i00, i10, i01)
            indices_.push_back(i00);
            indices_.push_back(i10);
            indices_.push_back(i01);

            // Triangle 2: (i10, i11, i01)
            indices_.push_back(i10);
            indices_.push_back(i11);
            indices_.push_back(i01);
        }
    }

    // Compute vertex normals via averaged face normals.
    for (std::size_t i = 0; i + 2 < indices_.size(); i += 3) {
        const QVector3D& v0 = vertices_[indices_[i]].position;
        const QVector3D& v1 = vertices_[indices_[i + 1]].position;
        const QVector3D& v2 = vertices_[indices_[i + 2]].position;

        QVector3D faceNormal = QVector3D::crossProduct(v1 - v0, v2 - v0);
        // Accumulate (area-weighted since cross product magnitude = 2*area).
        vertices_[indices_[i]].normal += faceNormal;
        vertices_[indices_[i + 1]].normal += faceNormal;
        vertices_[indices_[i + 2]].normal += faceNormal;
    }

    // Normalize all vertex normals.
    for (auto& v : vertices_) {
        float len = v.normal.length();
        if (len > 1e-7f)
            v.normal /= len;
        else
            v.normal = QVector3D(0, 0, 1);  // default up
    }
}

}  // namespace lumen::plot3d
