#pragma once

#include <QVector3D>

#include <cstdint>
#include <memory>
#include <vector>

namespace lumen::data {
class Volume3D;
}

namespace lumen::plot3d {

/// Marching Cubes mesh extraction (Lorensen & Cline 1987).
///
/// Given a Volume3D and an iso value, extracts a triangle mesh approximating
/// the isosurface. Uses the standard 256-entry edge/triangle lookup tables.
class MarchingCubes {
public:
    struct Vertex {
        QVector3D position;
        QVector3D normal;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        [[nodiscard]] std::size_t triangleCount() const
        {
            return indices.size() / 3;
        }

        [[nodiscard]] bool empty() const
        {
            return vertices.empty();
        }
    };

    /// Extract the isosurface mesh for a given iso value.
    static Mesh extract(const data::Volume3D& volume, double isoValue);

private:
    /// Interpolate vertex position along an edge.
    static QVector3D interpolateVertex(const QVector3D& p1, const QVector3D& p2,
                                        double v1, double v2, double isoValue);

    /// Compute vertex normal via central differences in the volume.
    static QVector3D computeGradient(const data::Volume3D& volume,
                                      std::size_t ix, std::size_t iy, std::size_t iz);

    /// The 256-entry edge table: for each cube configuration, a 12-bit mask
    /// indicating which edges are intersected.
    static const uint16_t kEdgeTable[256];

    /// The 256-entry triangle table: for each configuration, up to 5 triangles
    /// (15 edge indices, terminated by -1).
    static const int8_t kTriTable[256][16];
};

}  // namespace lumen::plot3d
