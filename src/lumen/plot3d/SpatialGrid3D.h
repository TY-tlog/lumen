#pragma once

#include <QVector3D>

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

namespace lumen::plot3d {

struct Ray;

/// Simple uniform spatial grid for accelerating ray-point queries in 3D.
///
/// Construction: insert point positions; they are bucketed into cells of
/// a uniform grid whose cell size is given at construction time.
/// Query: given a ray, perform DDA (digital differential analyzer) traversal
/// of grid cells, testing points in traversed cells for ray-sphere intersection.
class SpatialGrid3D {
public:
    /// Build the grid from a set of 3D points with a given cell size.
    SpatialGrid3D(const std::vector<QVector3D>& points, float cellSize);

    /// Result of a ray-sphere query against the grid.
    struct QueryResult {
        std::size_t pointIndex;
        float distance;
        QVector3D hitPoint;
    };

    /// Traverse the grid along \p ray, testing each point as a sphere of
    /// \p radius. Returns the closest hit within \p maxDist, or nullopt.
    [[nodiscard]] std::optional<QueryResult> queryRay(const Ray& ray,
                                                       float radius,
                                                       float maxDist) const;

    /// Number of occupied cells.
    [[nodiscard]] std::size_t occupiedCellCount() const { return cells_.size(); }

    /// Total number of indexed points.
    [[nodiscard]] std::size_t pointCount() const { return points_.size(); }

private:
    struct CellKey {
        int x, y, z;
        bool operator==(const CellKey& other) const = default;
    };

    struct CellKeyHash {
        std::size_t operator()(const CellKey& k) const
        {
            // Simple hash combining for 3 ints.
            std::size_t h = static_cast<std::size_t>(k.x) * 73856093u;
            h ^= static_cast<std::size_t>(k.y) * 19349663u;
            h ^= static_cast<std::size_t>(k.z) * 83492791u;
            return h;
        }
    };

    [[nodiscard]] CellKey cellOf(const QVector3D& p) const;

    /// Test ray-sphere intersection. Returns distance along ray, or -1.
    [[nodiscard]] static float raySphereIntersect(const QVector3D& rayOrigin,
                                                   const QVector3D& rayDir,
                                                   const QVector3D& center,
                                                   float radius);

    float cellSize_;
    std::vector<QVector3D> points_;
    std::unordered_map<CellKey, std::vector<std::size_t>, CellKeyHash> cells_;
};

}  // namespace lumen::plot3d
