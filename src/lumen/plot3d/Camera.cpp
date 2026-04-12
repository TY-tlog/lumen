#include "Camera.h"

#include <QJsonArray>

#include <algorithm>
#include <cmath>

namespace lumen::plot3d {

namespace {

constexpr float kMinDistance = 0.01f;
constexpr float kMaxElevation = 89.0f * (static_cast<float>(M_PI) / 180.0f);
constexpr float kArcballSensitivity = 0.005f;
constexpr float kOrbitSensitivity = 0.005f;
constexpr float kPanSensitivity = 0.005f;
constexpr float kZoomFactor = 0.001f;

}  // namespace

Camera::Camera()
    : orientation_(QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), 0))
{
}

QMatrix4x4 Camera::viewMatrix() const
{
    QVector3D eye = position();
    QVector3D upDir = up();
    QMatrix4x4 view;
    view.lookAt(eye, target_, upDir);
    return view;
}

QMatrix4x4 Camera::projectionMatrix(float aspect) const
{
    QMatrix4x4 proj;
    proj.perspective(fov_, aspect, near_, far_);
    return proj;
}

void Camera::setMode(CameraMode mode)
{
    if (mode_ == mode)
        return;

    if (mode_ == CameraMode::Trackball && mode == CameraMode::Orbit) {
        // Convert quaternion orientation to azimuth/elevation.
        QVector3D dir = orientation_.rotatedVector(QVector3D(0, 0, 1));
        azimuth_ = std::atan2(dir.x(), dir.z());
        elevation_ = std::asin(std::clamp(dir.y(), -1.0f, 1.0f));
    } else if (mode_ == CameraMode::Orbit && mode == CameraMode::Trackball) {
        // Convert azimuth/elevation to quaternion.
        QQuaternion yaw = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0),
                                                         azimuth_ * 180.0f / static_cast<float>(M_PI));
        QQuaternion pitch = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0),
                                                           -elevation_ * 180.0f / static_cast<float>(M_PI));
        orientation_ = yaw * pitch;
    }

    mode_ = mode;
}

void Camera::handleDrag(QPointF delta)
{
    if (mode_ == CameraMode::Trackball) {
        // Arcball: convert mouse delta to axis-angle rotation.
        float dx = static_cast<float>(delta.x()) * kArcballSensitivity;
        float dy = static_cast<float>(delta.y()) * kArcballSensitivity;

        float angle = std::sqrt(dx * dx + dy * dy);
        if (angle > 1e-6f) {
            // Rotation axis is perpendicular to the drag direction in screen space.
            QVector3D axis(-dy, dx, 0);
            axis.normalize();
            QQuaternion drag = QQuaternion::fromAxisAndAngle(axis, angle * 180.0f / static_cast<float>(M_PI));
            orientation_ = drag * orientation_;
            orientation_.normalize();
        }
    } else {
        // Orbit: delta.x -> azimuth, delta.y -> elevation.
        azimuth_ += static_cast<float>(delta.x()) * kOrbitSensitivity;
        elevation_ += static_cast<float>(delta.y()) * kOrbitSensitivity;
        elevation_ = std::clamp(elevation_, -kMaxElevation, kMaxElevation);
    }
}

void Camera::handleWheel(double delta)
{
    float factor = 1.0f - static_cast<float>(delta) * kZoomFactor;
    distance_ = std::max(kMinDistance, distance_ * factor);
}

void Camera::handlePan(QPointF delta)
{
    // Pan: translate target in the camera's local right/up plane.
    QVector3D right;
    QVector3D upDir;

    if (mode_ == CameraMode::Trackball) {
        right = orientation_.rotatedVector(QVector3D(1, 0, 0));
        upDir = orientation_.rotatedVector(QVector3D(0, 1, 0));
    } else {
        float cosEl = std::cos(elevation_);
        float sinEl = std::sin(elevation_);
        float cosAz = std::cos(azimuth_);
        float sinAz = std::sin(azimuth_);

        QVector3D forward(cosEl * sinAz, sinEl, cosEl * cosAz);
        QVector3D worldUp(0, 1, 0);
        right = QVector3D::crossProduct(forward, worldUp).normalized();
        upDir = QVector3D::crossProduct(right, forward).normalized();
    }

    float panScale = distance_ * kPanSensitivity;
    target_ += right * static_cast<float>(-delta.x()) * panScale;
    target_ += upDir * static_cast<float>(delta.y()) * panScale;
}

QVector3D Camera::position() const
{
    if (mode_ == CameraMode::Trackball) {
        QVector3D offset = orientation_.rotatedVector(QVector3D(0, 0, distance_));
        return target_ + offset;
    }

    // Orbit: spherical to cartesian.
    float cosEl = std::cos(elevation_);
    float sinEl = std::sin(elevation_);
    float cosAz = std::cos(azimuth_);
    float sinAz = std::sin(azimuth_);

    return target_ + QVector3D(cosEl * sinAz, sinEl, cosEl * cosAz) * distance_;
}

QVector3D Camera::up() const
{
    if (mode_ == CameraMode::Trackball) {
        return orientation_.rotatedVector(QVector3D(0, 1, 0));
    }
    // Orbit: up is always world-up (unless elevation is at poles, handled by clamp).
    return QVector3D(0, 1, 0);
}

void Camera::setPosition(QVector3D pos)
{
    QVector3D diff = pos - target_;
    distance_ = diff.length();
    if (distance_ < kMinDistance)
        distance_ = kMinDistance;

    QVector3D dir = diff.normalized();

    if (mode_ == CameraMode::Trackball) {
        // Compute quaternion that rotates (0,0,1) to dir.
        QVector3D from(0, 0, 1);
        orientation_ = QQuaternion::rotationTo(from, dir);
    } else {
        azimuth_ = std::atan2(dir.x(), dir.z());
        elevation_ = std::asin(std::clamp(dir.y(), -1.0f, 1.0f));
    }
}

void Camera::setTarget(QVector3D tgt)
{
    target_ = tgt;
}

void Camera::setFov(float degrees)
{
    fov_ = std::clamp(degrees, 1.0f, 179.0f);
}

void Camera::setNearFar(float nearVal, float farVal)
{
    near_ = nearVal;
    far_ = farVal;
}

QJsonObject Camera::toJson() const
{
    QJsonObject obj;
    obj[QStringLiteral("mode")] = (mode_ == CameraMode::Trackball)
                                      ? QStringLiteral("trackball")
                                      : QStringLiteral("orbit");
    obj[QStringLiteral("target")] = QJsonArray{target_.x(), target_.y(), target_.z()};
    obj[QStringLiteral("distance")] = static_cast<double>(distance_);
    obj[QStringLiteral("fov")] = static_cast<double>(fov_);
    obj[QStringLiteral("near")] = static_cast<double>(near_);
    obj[QStringLiteral("far")] = static_cast<double>(far_);

    // Save orientation for trackball.
    QJsonObject quat;
    quat[QStringLiteral("scalar")] = static_cast<double>(orientation_.scalar());
    quat[QStringLiteral("x")] = static_cast<double>(orientation_.x());
    quat[QStringLiteral("y")] = static_cast<double>(orientation_.y());
    quat[QStringLiteral("z")] = static_cast<double>(orientation_.z());
    obj[QStringLiteral("orientation")] = quat;

    obj[QStringLiteral("azimuth")] = static_cast<double>(azimuth_);
    obj[QStringLiteral("elevation")] = static_cast<double>(elevation_);

    return obj;
}

void Camera::fromJson(const QJsonObject& obj)
{
    QString modeStr = obj[QStringLiteral("mode")].toString();
    mode_ = (modeStr == QStringLiteral("orbit")) ? CameraMode::Orbit
                                                   : CameraMode::Trackball;

    QJsonArray tgtArr = obj[QStringLiteral("target")].toArray();
    if (tgtArr.size() == 3) {
        target_ = QVector3D(static_cast<float>(tgtArr[0].toDouble()),
                            static_cast<float>(tgtArr[1].toDouble()),
                            static_cast<float>(tgtArr[2].toDouble()));
    }

    distance_ = static_cast<float>(obj[QStringLiteral("distance")].toDouble(5.0));
    fov_ = static_cast<float>(obj[QStringLiteral("fov")].toDouble(45.0));
    near_ = static_cast<float>(obj[QStringLiteral("near")].toDouble(0.1));
    far_ = static_cast<float>(obj[QStringLiteral("far")].toDouble(1000.0));

    QJsonObject quat = obj[QStringLiteral("orientation")].toObject();
    if (!quat.isEmpty()) {
        orientation_ = QQuaternion(
            static_cast<float>(quat[QStringLiteral("scalar")].toDouble(1.0)),
            static_cast<float>(quat[QStringLiteral("x")].toDouble()),
            static_cast<float>(quat[QStringLiteral("y")].toDouble()),
            static_cast<float>(quat[QStringLiteral("z")].toDouble()));
    }

    azimuth_ = static_cast<float>(obj[QStringLiteral("azimuth")].toDouble());
    elevation_ = static_cast<float>(obj[QStringLiteral("elevation")].toDouble(0.3));
}

}  // namespace lumen::plot3d
