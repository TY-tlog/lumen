#include "ShaderProgram.h"

#include <QDebug>

namespace lumen::plot3d {

bool ShaderProgram::compile(const QString& vertexSource, const QString& fragmentSource)
{
    if (!program_.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource)) {
        qWarning() << "Vertex shader compilation failed:" << program_.log();
        return false;
    }

    if (!program_.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource)) {
        qWarning() << "Fragment shader compilation failed:" << program_.log();
        return false;
    }

    if (!program_.link()) {
        qWarning() << "Shader program linking failed:" << program_.log();
        return false;
    }

    return true;
}

void ShaderProgram::bind()
{
    program_.bind();
}

void ShaderProgram::release()
{
    program_.release();
}

void ShaderProgram::setUniform(const QString& name, const QMatrix4x4& mat)
{
    program_.setUniformValue(name.toUtf8().constData(), mat);
}

void ShaderProgram::setUniform(const QString& name, const QVector3D& vec)
{
    program_.setUniformValue(name.toUtf8().constData(), vec);
}

void ShaderProgram::setUniform(const QString& name, float val)
{
    program_.setUniformValue(name.toUtf8().constData(), val);
}

void ShaderProgram::setUniform(const QString& name, int val)
{
    program_.setUniformValue(name.toUtf8().constData(), val);
}

void ShaderProgram::setUniformMat3(const QString& name, const QMatrix3x3& mat)
{
    program_.setUniformValue(name.toUtf8().constData(), mat);
}

GLuint ShaderProgram::programId() const
{
    return program_.programId();
}

}  // namespace lumen::plot3d
