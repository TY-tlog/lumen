#include "Renderer3D.h"
#include "Camera.h"
#include "Light.h"
#include "PlotItem3D.h"
#include "Scene3D.h"

namespace lumen::plot3d {

namespace {

// GL version 410 for broad compatibility (macOS max is 4.1).
// Phase 8.1 embeds shaders as C++ string constants.

const char* kPhongVertexShader = R"glsl(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uNormalMatrix;
out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vColor;
void main() {
    vWorldPos = vec3(uModel * vec4(aPos, 1.0));
    vNormal = normalize(uNormalMatrix * aNormal);
    vColor = aColor;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)glsl";

const char* kPhongFragmentShader = R"glsl(
#version 410 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbient;
uniform vec3 uCameraPos;
uniform float uSpecularPower;
out vec4 fragColor;
void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 R = reflect(-L, N);
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(V, R), 0.0), uSpecularPower);
    vec3 color = vColor * (uAmbient + diff * uLightColor + spec * uLightColor * 0.5);
    fragColor = vec4(color, 1.0);
}
)glsl";

}  // namespace

bool Renderer3D::initialize()
{
    if (initialized_)
        return true;

    if (!phongShader_.compile(QString::fromLatin1(kPhongVertexShader),
                              QString::fromLatin1(kPhongFragmentShader))) {
        return false;
    }

    initialized_ = true;
    return true;
}

void Renderer3D::render(Scene3D& scene, Camera& camera, QSize viewport)
{
    if (!initialized_)
        return;

    float aspect = static_cast<float>(viewport.width()) /
                   static_cast<float>(std::max(1, viewport.height()));

    QMatrix4x4 view = camera.viewMatrix();
    QMatrix4x4 proj = camera.projectionMatrix(aspect);

    RenderContext ctx;
    ctx.viewMatrix = view;
    ctx.projectionMatrix = proj;
    ctx.vpMatrix = proj * view;
    ctx.viewport = viewport;
    ctx.lights = scene.lights();
    ctx.cameraPosition = camera.position();

    // Set up shader uniforms common to the scene.
    phongShader_.bind();

    // Find the first directional light for the Phong shader.
    QVector3D lightDir(0.5f, -1.0f, -0.5f);
    QVector3D lightColor(1.0f, 1.0f, 1.0f);
    QVector3D ambient(0.2f, 0.2f, 0.2f);

    for (const auto& light : scene.lights()) {
        if (light.type == LightType::Directional) {
            lightDir = light.direction.normalized();
            lightColor = light.color * light.intensity;
        } else if (light.type == LightType::Ambient) {
            ambient = light.color * light.intensity;
        }
    }

    phongShader_.setUniform(QStringLiteral("uLightDir"), lightDir);
    phongShader_.setUniform(QStringLiteral("uLightColor"), lightColor);
    phongShader_.setUniform(QStringLiteral("uAmbient"), ambient);
    phongShader_.setUniform(QStringLiteral("uCameraPos"), camera.position());
    phongShader_.setUniform(QStringLiteral("uSpecularPower"), 32.0f);

    // Render each visible item.
    for (const auto& item : scene.items()) {
        if (item && item->isVisible()) {
            renderItem(*item, phongShader_, ctx);
        }
    }

    phongShader_.release();
}

void Renderer3D::renderItem(PlotItem3D& item, ShaderProgram& shader,
                             const RenderContext& ctx)
{
    item.render(shader, ctx);
}

}  // namespace lumen::plot3d
