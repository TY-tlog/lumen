#include "SpatialGrid3D.h"
#include "Ray.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumen::plot3d {

SpatialGrid3D::SpatialGrid3D(const std::vector<QVector3D>& points, float cellSize)
    : cellSize_(cellSize)
    , points_(points)
{
    if (cellSize_ <= 0.0f)
        cellSize_ = 1.0f;

    for (std::size_t i = 0; i < points_.size(); ++i) {
        CellKey key = cellOf(points_[i]);
        cells_[key].push_back(i);
    }
}

SpatialGrid3D::CellKey SpatialGrid3D::cellOf(const QVector3D& p) const
{
    return CellKey{
        static_cast<int>(std::floor(p.x() / cellSize_)),
        static_cast<int>(std::floor(p.y() / cellSize_)),
        static_cast<int>(std::floor(p.z() / cellSize_))};
}

float SpatialGrid3D::raySphereIntersect(const QVector3D& rayOrigin,
                                          const QVector3D& rayDir,
                                          const QVector3D& center,
                                          float radius)
{
    QVector3D oc = rayOrigin - center;
    float a = QVector3D::dotProduct(rayDir, rayDir);
    float b = 2.0f * QVector3D::dotProduct(oc, rayDir);
    float c = QVector3D::dotProduct(oc, oc) - radius * radius;
    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f)
        return -1.0f;

    float sqrtD = std::sqrt(discriminant);
    float t1 = (-b - sqrtD) / (2.0f * a);
    float t2 = (-b + sqrtD) / (2.0f * a);

    // Return nearest positive intersection.
    if (t1 >= 0.0f)
        return t1;
    if (t2 >= 0.0f)
        return t2;
    return -1.0f;
}

std::optional<SpatialGrid3D::QueryResult> SpatialGrid3D::queryRay(
    const Ray& ray, float radius, float maxDist) const
{
    // DDA traversal of the uniform grid.
    // We expand the search by 1 cell in each direction to account for
    // spheres that straddle cell boundaries.

    // Determine the range of cells the ray can possibly traverse.
    // We use a conservative bounding box along the ray up to maxDist.
    QVector3D startPt = ray.origin;
    QVector3D endPt = ray.origin + ray.direction * maxDist;

    float minX = std::min(startPt.x(), endPt.x()) - radius;
    float minY = std::min(startPt.y(), endPt.y()) - radius;
    float minZ = std::min(startPt.z(), endPt.z()) - radius;
    float maxX = std::max(startPt.x(), endPt.x()) + radius;
    float maxY = std::max(startPt.y(), endPt.y()) + radius;
    float maxZ = std::max(startPt.z(), endPt.z()) + radius;

    int cellMinX = static_cast<int>(std::floor(minX / cellSize_));
    int cellMinY = static_cast<int>(std::floor(minY / cellSize_));
    int cellMinZ = static_cast<int>(std::floor(minZ / cellSize_));
    int cellMaxX = static_cast<int>(std::floor(maxX / cellSize_));
    int cellMaxY = static_cast<int>(std::floor(maxY / cellSize_));
    int cellMaxZ = static_cast<int>(std::floor(maxZ / cellSize_));

    std::optional<QueryResult> best;
    float bestDist = maxDist;

    for (int cx = cellMinX; cx <= cellMaxX; ++cx) {
        for (int cy = cellMinY; cy <= cellMaxY; ++cy) {
            for (int cz = cellMinZ; cz <= cellMaxZ; ++cz) {
                auto it = cells_.find(CellKey{cx, cy, cz});
                if (it == cells_.end())
                    continue;

                for (std::size_t idx : it->second) {
                    float t = raySphereIntersect(ray.origin, ray.direction,
                                                  points_[idx], radius);
                    if (t >= 0.0f && t < bestDist) {
                        bestDist = t;
                        best = QueryResult{idx, t,
                                           ray.origin + ray.direction * t};
                    }
                }
            }
        }
    }

    return best;
}

}  // namespace lumen::plot3d
