#pragma once

#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QString>
#include <QVector3D>

namespace lumen::plot3d {

class ShaderProgram {
public:
    bool compile(const QString& vertexSource, const QString& fragmentSource);
    void bind();
    void release();

    void setUniform(const QString& name, const QMatrix4x4& mat);
    void setUniform(const QString& name, const QVector3D& vec);
    void setUniform(const QString& name, float val);
    void setUniform(const QString& name, int val);
    void setUniformMat3(const QString& name, const QMatrix3x3& mat);

    [[nodiscard]] GLuint programId() const;

private:
    QOpenGLShaderProgram program_;
};

}  // namespace lumen::plot3d
