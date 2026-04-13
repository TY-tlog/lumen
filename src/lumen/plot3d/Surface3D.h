#pragma once

#include "PlotItem3D.h"

#include <QColor>
#include <QString>
#include <QVector3D>

#include <memory>
#include <optional>
#include <vector>

namespace lumen::data {
class Grid2D;
}

namespace lumen::plot {
class Colormap;
}

namespace lumen::plot3d {

/// 3D surface plot item. Triangulates a Grid2D into a renderable mesh.
///
/// Three display modes:
/// - HeightMap: z-coordinate equals the grid value; colored by colormap.
/// - FlatColored: z=0 flat surface with colormap colors.
/// - Both: height mapping with colormap overlay.
///
/// Vertex normals are computed by averaging face normals of adjacent triangles.
class Surface3D : public PlotItem3D {
public:
    enum class Mode { HeightMap, FlatColored, Both };

    explicit Surface3D(std::shared_ptr<data::Grid2D> grid,
                       QColor baseColor = QColor(100, 149, 237),
                       QString name = {});

    ~Surface3D() override;

    Type type() const override { return Type::Surface3D; }
    BoundingBox3D dataBounds() const override;
    void render(ShaderProgram& shader, const RenderContext& ctx) override;
    std::optional<HitResult3D> hitTestRay(const Ray& ray,
                                           double maxDist) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    void invalidate() override;

    // Mode
    void setMode(Mode mode);
    [[nodiscard]] Mode mode() const { return mode_; }

    // Wireframe
    void setWireframe(bool enabled);
    [[nodiscard]] bool wireframe() const { return wireframe_; }

    // Colormap
    void setColormap(std::shared_ptr<plot::Colormap> cmap);

    // Accessors for testing
    void setName(QString name);
    void setVisible(bool v);
    void setBaseColor(QColor c);

    /// Access to the built mesh data (for testing).
    struct Vertex {
        QVector3D position;
        QVector3D normal;
        QVector3D color;
    };

    [[nodiscard]] const std::vector<Vertex>& vertices() const;
    [[nodiscard]] const std::vector<uint32_t>& indices() const;

    /// Number of triangles in the mesh.
    [[nodiscard]] std::size_t triangleCount() const;

private:
    void buildMesh() const;

    std::shared_ptr<data::Grid2D> grid_;
    std::shared_ptr<plot::Colormap> colormap_;
    Mode mode_ = Mode::HeightMap;
    bool wireframe_ = false;
    QColor baseColor_;
    QString name_;
    bool visible_ = true;

    mutable bool meshDirty_ = true;
    mutable std::vector<Vertex> vertices_;
    mutable std::vector<uint32_t> indices_;
};

}  // namespace lumen::plot3d
