#include "PlotCanvas3D.h"

#include <QDebug>
#include <QSurfaceFormat>
#include <QtMath>

namespace lumen::ui {

PlotCanvas3D::PlotCanvas3D(QWidget* parent)
    : QOpenGLWidget(parent)
    , scene_(std::make_unique<plot3d::Scene3D>())
    , camera_(std::make_unique<plot3d::Camera>())
    , renderer_(std::make_unique<plot3d::Renderer3D>())
{
    scene_->addDefaultLights();
    setMouseTracking(true);
}

PlotCanvas3D::~PlotCanvas3D() = default;

void PlotCanvas3D::setCameraMode(plot3d::CameraMode mode)
{
    camera_->setMode(mode);
    update();
}

void PlotCanvas3D::addItem(std::unique_ptr<plot3d::PlotItem3D> item)
{
    scene_->addItem(std::move(item));
    fitCamera();
    update();
}

void PlotCanvas3D::fitCamera()
{
    auto bounds = scene_->sceneBounds();
    if (!bounds.isValid())
        return;

    QVector3D center = bounds.center();
    QVector3D sz = bounds.size();
    float maxDim = std::max({sz.x(), sz.y(), sz.z(), 0.001f});
    float radius = maxDim * 0.5f * std::sqrt(3.0f);

    camera_->setTarget(center);
    float dist = radius / std::tan(qDegreesToRadians(camera_->fov() * 0.5f));
    camera_->setNearFar(dist * 0.01f, dist * 100.0f);

    float d = dist * 1.5f;
    QVector3D eye = center + QVector3D(d * 0.5f, d * 0.35f, d * 0.75f);
    camera_->setPosition(eye);
}

void PlotCanvas3D::initializeGL()
{
    initializeOpenGLFunctions();

    qInfo() << "PlotCanvas3D: GL context created, version:"
            << reinterpret_cast<const char*>(glGetString(GL_VERSION));

    if (!renderer_->initialize()) {
        qWarning() << "PlotCanvas3D: renderer initialization failed";
        return;
    }

    qInfo() << "PlotCanvas3D: renderer initialized successfully";
    glInitialized_ = true;
}

void PlotCanvas3D::paintGL()
{
    if (!glInitialized_)
        return;

    glClearColor(0.18f, 0.20f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(0x8642);   // GL_PROGRAM_POINT_SIZE
    glEnable(0x0BE2);   // GL_BLEND
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    renderer_->render(*scene_, *camera_, size());
}

void PlotCanvas3D::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
    // The viewport is set by render() via the passed size().
}

void PlotCanvas3D::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragging_ = true;
        lastMousePos_ = event->pos();
        event->accept();
    } else {
        QOpenGLWidget::mousePressEvent(event);
    }
}

void PlotCanvas3D::mouseMoveEvent(QMouseEvent* event)
{
    if (dragging_) {
        QPoint delta = event->pos() - lastMousePos_;
        lastMousePos_ = event->pos();

        if (event->modifiers() & Qt::ShiftModifier) {
            // Shift+drag: pan
            camera_->handlePan(QPointF(delta));
        } else {
            // Normal drag: rotate
            camera_->handleDrag(QPointF(delta));
        }
        update();
        event->accept();
    } else {
        QOpenGLWidget::mouseMoveEvent(event);
    }
}

void PlotCanvas3D::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
        event->accept();
    } else {
        QOpenGLWidget::mouseReleaseEvent(event);
    }
}

void PlotCanvas3D::wheelEvent(QWheelEvent* event)
{
    double delta = event->angleDelta().y();
    camera_->handleWheel(delta);
    update();
    event->accept();
}

void PlotCanvas3D::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QOpenGLWidget::mouseDoubleClickEvent(event);
        return;
    }

    // Ray-cast through items to find the closest hit.
    plot3d::Ray ray = plot3d::Ray::fromScreenPixel(event->pos(), *camera_, size());

    double bestDist = 1000.0;  // maxDist = camera far plane
    int bestIndex = -1;
    plot3d::HitResult3D bestHit;

    const auto& items = scene_->items();
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        if (!items[static_cast<std::size_t>(i)] ||
            !items[static_cast<std::size_t>(i)]->isVisible())
            continue;

        auto hit = items[static_cast<std::size_t>(i)]->hitTestRay(ray, bestDist);
        if (hit && hit->distance < bestDist) {
            bestDist = hit->distance;
            bestHit = *hit;
            bestHit.itemIndex = i;
            bestIndex = i;
        }
    }

    if (bestIndex >= 0) {
        emit itemDoubleClicked(bestIndex, bestHit);
    }

    event->accept();
}

}  // namespace lumen::ui
