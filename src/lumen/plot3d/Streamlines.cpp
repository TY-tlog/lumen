#include "Streamlines.h"
#include "Ray.h"
#include "ShaderProgram.h"

#include "data/Volume3D.h"
#include "plot/Colormap.h"

#include <QMatrix4x4>

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumen::plot3d {

Streamlines::Streamlines(std::shared_ptr<data::Volume3D> vx,
                         std::shared_ptr<data::Volume3D> vy,
                         std::shared_ptr<data::Volume3D> vz,
                         QString name)
    : vx_(std::move(vx))
    , vy_(std::move(vy))
    , vz_(std::move(vz))
    , name_(std::move(name))
{
}

Streamlines::~Streamlines() = default;

BoundingBox3D Streamlines::dataBounds() const
{
    if (!vx_)
        return BoundingBox3D{};

    auto dims = vx_->dimensions();
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

void Streamlines::render(ShaderProgram& shader, const RenderContext& ctx)
{
    const auto& streamlines = lines();
    if (streamlines.empty())
        return;

    QMatrix4x4 model;
    QMatrix4x4 mvp = ctx.vpMatrix * model;
    shader.setUniform(QStringLiteral("uMVP"), mvp);
    shader.setUniform(QStringLiteral("uModel"), model);
    QMatrix3x3 normalMat = model.normalMatrix();
    shader.setUniformMat3(QStringLiteral("uNormalMatrix"), normalMat);

    // Build line strip vertex data for each streamline.
    float r = color_.redF();
    float g = color_.greenF();
    float b = color_.blueF();

    for (const auto& line : streamlines) {
        std::vector<float> vertexData;
        vertexData.reserve(line.points.size() * 9);

        for (std::size_t i = 0; i < line.points.size(); ++i) {
            const auto& p = line.points[i];
            vertexData.push_back(p.x());
            vertexData.push_back(p.y());
            vertexData.push_back(p.z());
            // Normal: tangent approximation.
            QVector3D tangent(0, 1, 0);
            if (i + 1 < line.points.size())
                tangent = (line.points[i + 1] - p).normalized();
            else if (i > 0)
                tangent = (p - line.points[i - 1]).normalized();
            vertexData.push_back(tangent.x());
            vertexData.push_back(tangent.y());
            vertexData.push_back(tangent.z());

            // Color by magnitude if colormap available.
            if (colormap_ && !line.magnitudes.empty()) {
                float maxMag = *std::max_element(line.magnitudes.begin(),
                                                  line.magnitudes.end());
                float t = (maxMag > 1e-7f) ? line.magnitudes[i] / maxMag : 0.0f;
                QColor c = colormap_->sample(static_cast<double>(t));
                vertexData.push_back(c.redF());
                vertexData.push_back(c.greenF());
                vertexData.push_back(c.blueF());
            } else {
                vertexData.push_back(r);
                vertexData.push_back(g);
                vertexData.push_back(b);
            }
        }

        // GL_LINE_STRIP rendering deferred to GL context.
        Q_UNUSED(vertexData);
    }
}

std::optional<HitResult3D> Streamlines::hitTestRay(const Ray& ray,
                                                      double maxDist) const
{
    // Hit-test each line segment of each streamline.
    const auto& streamlines = lines();
    float bestDist = static_cast<float>(maxDist);
    std::optional<HitResult3D> bestHit;

    const float lineRadius = 0.02f;  // hit tolerance

    for (const auto& line : streamlines) {
        for (std::size_t i = 0; i + 1 < line.points.size(); ++i) {
            const QVector3D& a = line.points[i];
            const QVector3D& b = line.points[i + 1];
            QVector3D ab = b - a;
            QVector3D ao = ray.origin - a;

            float abDotAb = QVector3D::dotProduct(ab, ab);
            float abDotDir = QVector3D::dotProduct(ab, ray.direction);
            float abDotAo = QVector3D::dotProduct(ab, ao);
            float dirDotDir = QVector3D::dotProduct(ray.direction, ray.direction);
            float dirDotAo = QVector3D::dotProduct(ray.direction, ao);

            float denom = abDotAb * dirDotDir - abDotDir * abDotDir;
            if (std::abs(denom) < 1e-10f)
                continue;

            float t = (abDotAb * dirDotAo - abDotDir * abDotAo) / denom;
            float s = (abDotDir * dirDotAo - dirDotDir * abDotAo) / denom;

            if (t < 0.0f || t >= bestDist || s < 0.0f || s > 1.0f)
                continue;

            QVector3D closestOnRay = ray.origin + ray.direction * t;
            QVector3D closestOnSeg = a + ab * s;
            float dist = (closestOnRay - closestOnSeg).length();

            if (dist < lineRadius && t < bestDist) {
                bestDist = t;
                HitResult3D hit;
                hit.itemIndex = -1;
                hit.distance = static_cast<double>(t);
                hit.worldPoint = closestOnRay;
                bestHit = hit;
            }
        }
    }

    return bestHit;
}

void Streamlines::invalidate()
{
    linesDirty_ = true;
}

void Streamlines::setSeedPoints(std::vector<QVector3D> seeds)
{
    seeds_ = std::move(seeds);
    linesDirty_ = true;
}

void Streamlines::setSeedGrid(int resolution)
{
    seeds_.clear();
    BoundingBox3D bounds = dataBounds();
    if (!bounds.isValid() || resolution < 1) {
        linesDirty_ = true;
        return;
    }

    QVector3D sz = bounds.size();
    for (int iz = 0; iz < resolution; ++iz) {
        for (int iy = 0; iy < resolution; ++iy) {
            for (int ix = 0; ix < resolution; ++ix) {
                float fx = (resolution > 1) ? static_cast<float>(ix) / static_cast<float>(resolution - 1) : 0.5f;
                float fy = (resolution > 1) ? static_cast<float>(iy) / static_cast<float>(resolution - 1) : 0.5f;
                float fz = (resolution > 1) ? static_cast<float>(iz) / static_cast<float>(resolution - 1) : 0.5f;

                seeds_.emplace_back(
                    bounds.min.x() + fx * sz.x(),
                    bounds.min.y() + fy * sz.y(),
                    bounds.min.z() + fz * sz.z());
            }
        }
    }

    linesDirty_ = true;
}

void Streamlines::setIntegrationStep(float h)
{
    stepSize_ = h;
    linesDirty_ = true;
}

void Streamlines::setMaxSteps(int steps)
{
    maxSteps_ = steps;
    linesDirty_ = true;
}

void Streamlines::setColor(QColor color)
{
    color_ = std::move(color);
}

void Streamlines::setColormap(std::shared_ptr<plot::Colormap> cmap)
{
    colormap_ = std::move(cmap);
}

void Streamlines::setName(QString name)
{
    name_ = std::move(name);
}

void Streamlines::setVisible(bool v)
{
    visible_ = v;
}

const std::vector<Streamlines::Line>& Streamlines::lines() const
{
    if (linesDirty_)
        buildStreamlines();
    return lines_;
}

QVector3D Streamlines::sampleField(const QVector3D& p) const
{
    if (!vx_ || !vy_ || !vz_)
        return QVector3D(0, 0, 0);

    auto dims = vx_->dimensions();
    if (dims.size() < 3)
        return QVector3D(0, 0, 0);

    std::size_t nx = vx_->sizeX();
    std::size_t ny = vx_->sizeY();
    std::size_t nz = vx_->sizeZ();

    if (nx < 2 || ny < 2 || nz < 2)
        return QVector3D(0, 0, 0);

    // Map world position to fractional grid coordinates.
    double x0 = dims[0].coordinates.valueAt(0);
    double x1 = dims[0].coordinates.valueAt(nx - 1);
    double y0 = dims[1].coordinates.valueAt(0);
    double y1 = dims[1].coordinates.valueAt(ny - 1);
    double z0 = dims[2].coordinates.valueAt(0);
    double z1 = dims[2].coordinates.valueAt(nz - 1);

    double fx = (x1 > x0) ? (static_cast<double>(p.x()) - x0) / (x1 - x0) * static_cast<double>(nx - 1) : 0.0;
    double fy = (y1 > y0) ? (static_cast<double>(p.y()) - y0) / (y1 - y0) * static_cast<double>(ny - 1) : 0.0;
    double fz = (z1 > z0) ? (static_cast<double>(p.z()) - z0) / (z1 - z0) * static_cast<double>(nz - 1) : 0.0;

    // Clamp to valid range.
    fx = std::clamp(fx, 0.0, static_cast<double>(nx - 1) - 1e-10);
    fy = std::clamp(fy, 0.0, static_cast<double>(ny - 1) - 1e-10);
    fz = std::clamp(fz, 0.0, static_cast<double>(nz - 1) - 1e-10);

    std::size_t ix = static_cast<std::size_t>(fx);
    std::size_t iy = static_cast<std::size_t>(fy);
    std::size_t iz = static_cast<std::size_t>(fz);

    if (ix >= nx - 1) ix = nx - 2;
    if (iy >= ny - 1) iy = ny - 2;
    if (iz >= nz - 1) iz = nz - 2;

    double dx = fx - static_cast<double>(ix);
    double dy = fy - static_cast<double>(iy);
    double dz = fz - static_cast<double>(iz);

    // Trilinear interpolation for each component.
    auto trilinear = [&](const data::Volume3D& vol) -> double {
        double c000 = vol.valueAt({ix, iy, iz});
        double c100 = vol.valueAt({ix + 1, iy, iz});
        double c010 = vol.valueAt({ix, iy + 1, iz});
        double c110 = vol.valueAt({ix + 1, iy + 1, iz});
        double c001 = vol.valueAt({ix, iy, iz + 1});
        double c101 = vol.valueAt({ix + 1, iy, iz + 1});
        double c011 = vol.valueAt({ix, iy + 1, iz + 1});
        double c111 = vol.valueAt({ix + 1, iy + 1, iz + 1});

        double c00 = c000 * (1 - dx) + c100 * dx;
        double c01 = c001 * (1 - dx) + c101 * dx;
        double c10 = c010 * (1 - dx) + c110 * dx;
        double c11 = c011 * (1 - dx) + c111 * dx;

        double c0 = c00 * (1 - dy) + c10 * dy;
        double c1 = c01 * (1 - dy) + c11 * dy;

        return c0 * (1 - dz) + c1 * dz;
    };

    return QVector3D(static_cast<float>(trilinear(*vx_)),
                     static_cast<float>(trilinear(*vy_)),
                     static_cast<float>(trilinear(*vz_)));
}

bool Streamlines::isInsideDomain(const QVector3D& p) const
{
    BoundingBox3D bounds = dataBounds();
    return p.x() >= bounds.min.x() && p.x() <= bounds.max.x() &&
           p.y() >= bounds.min.y() && p.y() <= bounds.max.y() &&
           p.z() >= bounds.min.z() && p.z() <= bounds.max.z();
}

QVector3D Streamlines::rk4Step(const QVector3D& p, float h) const
{
    QVector3D k1 = sampleField(p);
    QVector3D k2 = sampleField(p + k1 * (h * 0.5f));
    QVector3D k3 = sampleField(p + k2 * (h * 0.5f));
    QVector3D k4 = sampleField(p + k3 * h);

    return p + (k1 + k2 * 2.0f + k3 * 2.0f + k4) * (h / 6.0f);
}

void Streamlines::buildStreamlines() const
{
    lines_.clear();
    linesDirty_ = false;

    if (!vx_ || !vy_ || !vz_ || seeds_.empty())
        return;

    domainBounds_ = dataBounds();

    for (const auto& seed : seeds_) {
        if (!isInsideDomain(seed))
            continue;

        Line line;
        QVector3D p = seed;
        line.points.push_back(p);

        QVector3D vel = sampleField(p);
        line.magnitudes.push_back(vel.length());

        for (int step = 0; step < maxSteps_; ++step) {
            QVector3D pNext = rk4Step(p, stepSize_);

            if (!isInsideDomain(pNext))
                break;

            // Check for convergence (velocity near zero).
            QVector3D v = sampleField(pNext);
            float mag = v.length();
            if (mag < 1e-8f)
                break;

            line.points.push_back(pNext);
            line.magnitudes.push_back(mag);
            p = pNext;
        }

        if (line.points.size() >= 2)
            lines_.push_back(std::move(line));
    }
}

}  // namespace lumen::plot3d
