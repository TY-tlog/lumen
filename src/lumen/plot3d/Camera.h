#pragma once

#include <QJsonObject>
#include <QMatrix4x4>
#include <QPointF>
#include <QQuaternion>
#include <QVector3D>

namespace lumen::plot3d {

enum class CameraMode { Trackball, Orbit };

class Camera {
public:
    Camera();

    [[nodiscard]] QMatrix4x4 viewMatrix() const;
    [[nodiscard]] QMatrix4x4 projectionMatrix(float aspect) const;

    void setMode(CameraMode mode);
    [[nodiscard]] CameraMode mode() const { return mode_; }

    // Interaction
    void handleDrag(QPointF delta);    // Trackball: arcball, Orbit: yaw/pitch
    void handleWheel(double delta);    // dolly zoom
    void handlePan(QPointF delta);     // translate target

    // State
    [[nodiscard]] QVector3D position() const;
    [[nodiscard]] QVector3D target() const { return target_; }
    [[nodiscard]] QVector3D up() const;
    void setPosition(QVector3D pos);
    void setTarget(QVector3D tgt);
    void setFov(float degrees);
    void setNearFar(float near, float far);

    [[nodiscard]] float distance() const { return distance_; }
    [[nodiscard]] float fov() const { return fov_; }
    [[nodiscard]] float nearPlane() const { return near_; }
    [[nodiscard]] float farPlane() const { return far_; }

    // Persistence
    [[nodiscard]] QJsonObject toJson() const;
    void fromJson(const QJsonObject& obj);

private:
    CameraMode mode_ = CameraMode::Trackball;
    QVector3D target_{0, 0, 0};
    float distance_ = 5.0f;
    float fov_ = 45.0f;
    float near_ = 0.1f;
    float far_ = 1000.0f;

    // Trackball: quaternion orientation
    QQuaternion orientation_;

    // Orbit: azimuth + elevation
    float azimuth_ = 0.0f;     // radians
    float elevation_ = 0.3f;   // radians
};

}  // namespace lumen::plot3d
