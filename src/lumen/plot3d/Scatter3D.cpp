#include "Scatter3D.h"
#include "Ray.h"
#include "ShaderProgram.h"

#include "data/Rank1Dataset.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumen::plot3d {

Scatter3D::Scatter3D(std::shared_ptr<data::Rank1Dataset> xData,
                     std::shared_ptr<data::Rank1Dataset> yData,
                     std::shared_ptr<data::Rank1Dataset> zData,
                     QColor color, QString name)
    : xData_(std::move(xData))
    , yData_(std::move(yData))
    , zData_(std::move(zData))
    , color_(std::move(color))
    , name_(std::move(name))
{
}

Scatter3D::~Scatter3D() = default;

BoundingBox3D Scatter3D::dataBounds() const
{
    const auto& pts = pointPositions();
    if (pts.empty())
        return BoundingBox3D{};

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& p : pts) {
        minX = std::min(minX, p.x());
        minY = std::min(minY, p.y());
        minZ = std::min(minZ, p.z());
        maxX = std::max(maxX, p.x());
        maxY = std::max(maxY, p.y());
        maxZ = std::max(maxZ, p.z());
    }

    return BoundingBox3D{QVector3D(minX, minY, minZ),
                          QVector3D(maxX, maxY, maxZ)};
}

void Scatter3D::render(ShaderProgram& shader, const RenderContext& ctx)
{
    const auto& pts = pointPositions();
    if (pts.empty())
        return;

    QMatrix4x4 model;
    QMatrix4x4 mvp = ctx.vpMatrix * model;
    shader.setUniform(QStringLiteral("uMVP"), mvp);
    shader.setUniform(QStringLiteral("uModel"), model);

    QMatrix3x3 normalMat = model.normalMatrix();
    shader.setUniformMat3(QStringLiteral("uNormalMatrix"), normalMat);

    auto* f = QOpenGLContext::currentContext()->functions();

    if (gpuDirty_ || !vao_.isCreated()) {
        float r = color_.redF();
        float g = color_.greenF();
        float b = color_.blueF();

        std::vector<float> vertexData;
        vertexData.reserve(pts.size() * 9);
        for (const auto& p : pts) {
            vertexData.push_back(p.x());
            vertexData.push_back(p.y());
            vertexData.push_back(p.z());
            vertexData.push_back(0.0f);
            vertexData.push_back(1.0f);
            vertexData.push_back(0.0f);
            vertexData.push_back(r);
            vertexData.push_back(g);
            vertexData.push_back(b);
        }

        if (!vao_.isCreated()) {
            vao_.create();
            vbo_.create();
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

        vbo_.release();
        vao_.release();

        gpuVertexCount_ = static_cast<int>(pts.size());
        gpuDirty_ = false;
    }

    vao_.bind();
    f->glDrawArrays(GL_POINTS, 0, gpuVertexCount_);
    vao_.release();
}

std::optional<HitResult3D> Scatter3D::hitTestRay(const Ray& ray,
                                                   double maxDist) const
{
    const auto& pts = pointPositions();
    if (pts.empty())
        return std::nullopt;

    float maxDistF = static_cast<float>(maxDist);

    // Use spatial grid for large datasets.
    if (pts.size() >= kSpatialGridThreshold) {
        buildSpatialGrid();
        if (spatialGrid_) {
            auto result = spatialGrid_->queryRay(ray, markerSize_, maxDistF);
            if (result) {
                HitResult3D hit;
                hit.itemIndex = -1;  // caller sets this
                hit.distance = static_cast<double>(result->distance);
                hit.worldPoint = result->hitPoint;
                return hit;
            }
            return std::nullopt;
        }
    }

    // Brute-force ray-sphere test for smaller datasets.
    float bestDist = maxDistF;
    std::optional<HitResult3D> bestHit;

    for (const auto& p : pts) {
        QVector3D oc = ray.origin - p;
        float a = QVector3D::dotProduct(ray.direction, ray.direction);
        float b = 2.0f * QVector3D::dotProduct(oc, ray.direction);
        float c = QVector3D::dotProduct(oc, oc) - markerSize_ * markerSize_;
        float discriminant = b * b - 4.0f * a * c;

        if (discriminant < 0.0f)
            continue;

        float sqrtD = std::sqrt(discriminant);
        float t = (-b - sqrtD) / (2.0f * a);
        if (t < 0.0f)
            t = (-b + sqrtD) / (2.0f * a);
        if (t < 0.0f || t >= bestDist)
            continue;

        bestDist = t;
        HitResult3D hit;
        hit.itemIndex = -1;  // caller sets this
        hit.distance = static_cast<double>(t);
        hit.worldPoint = ray.pointAt(static_cast<double>(t));
        bestHit = hit;
    }

    return bestHit;
}

void Scatter3D::invalidate()
{
    positionsDirty_ = true;
    gpuDirty_ = true;
    spatialGrid_.reset();
}

void Scatter3D::setMarkerShape(MarkerShape3D shape)
{
    shape_ = shape;
}

void Scatter3D::setMarkerSize(float worldUnits)
{
    markerSize_ = worldUnits;
    // Invalidate spatial grid since radius affects queries.
    spatialGrid_.reset();
}

void Scatter3D::setColor(QColor color)
{
    color_ = std::move(color);
    gpuDirty_ = true;
}

void Scatter3D::setName(QString name)
{
    name_ = std::move(name);
}

void Scatter3D::setVisible(bool v)
{
    visible_ = v;
}

const std::vector<QVector3D>& Scatter3D::pointPositions() const
{
    if (positionsDirty_) {
        buildPositionCache();
    }
    return positions_;
}

void Scatter3D::buildPositionCache() const
{
    positions_.clear();
    spatialGrid_.reset();

    if (!xData_ || !yData_ || !zData_)
        return;

    std::size_t n = std::min({xData_->rowCount(),
                              yData_->rowCount(),
                              zData_->rowCount()});

    positions_.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        float x = static_cast<float>(xData_->valueAt({i}));
        float y = static_cast<float>(yData_->valueAt({i}));
        float z = static_cast<float>(zData_->valueAt({i}));
        positions_.emplace_back(x, y, z);
    }

    positionsDirty_ = false;
}

void Scatter3D::buildSpatialGrid() const
{
    if (spatialGrid_)
        return;

    const auto& pts = pointPositions();
    if (pts.empty())
        return;

    // Cell size: a reasonable multiple of marker size for good bucket distribution.
    float cellSize = markerSize_ * 10.0f;
    if (cellSize <= 0.0f)
        cellSize = 1.0f;

    spatialGrid_ = std::make_unique<SpatialGrid3D>(pts, cellSize);
}

}  // namespace lumen::plot3d
