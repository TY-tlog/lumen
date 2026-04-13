#pragma once

#include "plot3d/Camera.h"
#include "plot3d/PlotItem3D.h"
#include "plot3d/Ray.h"
#include "plot3d/RenderContext.h"
#include "plot3d/Renderer3D.h"
#include "plot3d/Scene3D.h"

#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QWheelEvent>

#include <memory>

namespace lumen::ui {

/// QOpenGLWidget subclass that hosts a 3D plot scene.
///
/// Owns a Scene3D, Camera, and Renderer3D. Handles mouse interaction
/// (drag for camera rotation, wheel for zoom, double-click for picking).
/// GL version-specific functions (4.1/4.5) are accessed through
/// QOpenGLContext::versionFunctions() in initializeGL rather than
/// through multiple inheritance, avoiding MOC compatibility issues.
class PlotCanvas3D : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit PlotCanvas3D(QWidget* parent = nullptr);
    ~PlotCanvas3D() override;

    [[nodiscard]] plot3d::Scene3D* scene() { return scene_.get(); }
    [[nodiscard]] plot3d::Camera* camera() { return camera_.get(); }
    void setCameraMode(plot3d::CameraMode mode);

    void addItem(std::unique_ptr<plot3d::PlotItem3D> item);

    [[nodiscard]] bool isGLInitialized() const { return glInitialized_; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

signals:
    void itemDoubleClicked(int itemIndex, lumen::plot3d::HitResult3D hit);

private:
    std::unique_ptr<plot3d::Scene3D> scene_;
    std::unique_ptr<plot3d::Camera> camera_;
    std::unique_ptr<plot3d::Renderer3D> renderer_;
    QPoint lastMousePos_;
    bool dragging_ = false;
    bool glInitialized_ = false;
};

}  // namespace lumen::ui
