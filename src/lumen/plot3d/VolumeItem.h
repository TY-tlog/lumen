#pragma once

#include "PlotItem3D.h"
#include "TransferFunction.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QString>

#include <memory>

namespace lumen::data {
class Volume3D;
}

namespace lumen::plot3d {

/// 3D volume rendering item using texture slicing (Phase 8).
///
/// Renders a 3D volume as a series of semi-transparent slices
/// (proxy cube faces). A TransferFunction maps scalar values to RGBA.
class VolumeItem : public PlotItem3D {
public:
    explicit VolumeItem(std::shared_ptr<data::Volume3D> volume,
                        QString name = {});

    ~VolumeItem() override;

    Type type() const override { return Type::Volume; }
    BoundingBox3D dataBounds() const override;
    void render(ShaderProgram& shader, const RenderContext& ctx) override;
    std::optional<HitResult3D> hitTestRay(const Ray& ray,
                                           double maxDist) const override;
    bool isVisible() const override { return visible_; }
    QString name() const override { return name_; }
    void invalidate() override;

    // Transfer function
    void setTransferFunction(TransferFunction tf);
    [[nodiscard]] const TransferFunction& transferFunction() const
    {
        return transferFunction_;
    }

    // Sampling parameters
    void setSampleStep(float step);
    [[nodiscard]] float sampleStep() const { return sampleStep_; }

    void setMaxSamples(int count);
    [[nodiscard]] int maxSamples() const { return maxSamples_; }

    // Accessors
    void setName(QString name);
    void setVisible(bool v);

    [[nodiscard]] const std::shared_ptr<data::Volume3D>& volume() const
    {
        return volume_;
    }

private:
    std::shared_ptr<data::Volume3D> volume_;
    TransferFunction transferFunction_;
    float sampleStep_ = 0.01f;
    int maxSamples_ = 256;
    QString name_;
    bool visible_ = true;

    mutable QOpenGLVertexArrayObject vao_;
    mutable QOpenGLBuffer vbo_{QOpenGLBuffer::VertexBuffer};
    mutable QOpenGLBuffer ebo_{QOpenGLBuffer::IndexBuffer};
    mutable bool gpuDirty_ = true;
};

}  // namespace lumen::plot3d
