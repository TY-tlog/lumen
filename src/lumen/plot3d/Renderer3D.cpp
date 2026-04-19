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
    gl_PointSize = 8.0;
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

// Point fragment shader: renders GL_POINTS as lit spheres.
const char* kPointFragmentShader = R"glsl(
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
    vec2 pc = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(pc, pc);
    if (r2 > 1.0) discard;
    float z = sqrt(1.0 - r2);
    vec3 sphereNormal = vec3(pc, z);
    vec3 L = normalize(-uLightDir);
    float diff = max(dot(sphereNormal, L), 0.0);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 R = reflect(-L, sphereNormal);
    float spec = pow(max(dot(V, R), 0.0), uSpecularPower) * 0.4;
    vec3 color = vColor * (uAmbient + diff * uLightColor) + spec * uLightColor;
    float edge = smoothstep(0.7, 1.0, sqrt(r2));
    fragColor = vec4(color * (1.0 - edge * 0.25), 1.0 - edge * 0.3);
}
)glsl";

// PBR vertex shader: same as Phong (position, normal, color passthrough).
const char* kPbrVertexShader = R"glsl(
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
    gl_PointSize = 8.0;
}
)glsl";

// PBR fragment shader: Cook-Torrance BRDF.
const char* kPbrFragmentShader = R"glsl(
#version 410 core

const float PI = 3.14159265359;

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vColor;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbient;
uniform vec3 uCameraPos;

// PBR material uniforms.
uniform vec3 uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform float uIor;
uniform vec3 uEmissive;

out vec4 fragColor;

// GGX/Trowbridge-Reitz normal distribution function.
float distributionGGX(vec3 N, vec3 H, float a) {
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

// Smith geometry function (single direction).
float geometrySmithG1(vec3 N, vec3 X, float a) {
    float NdotX = max(dot(N, X), 0.0);
    float a2 = a * a;
    return 2.0 * NdotX / (NdotX + sqrt(a2 + (1.0 - a2) * NdotX * NdotX));
}

// Smith geometry function (combined).
float geometrySmith(vec3 N, vec3 V, vec3 L, float a) {
    return geometrySmithG1(N, V, a) * geometrySmithG1(N, L, a);
}

// Fresnel-Schlick approximation.
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);

    // Base reflectivity from IOR (Fresnel at normal incidence).
    float f0_scalar = (uIor - 1.0) / (uIor + 1.0);
    f0_scalar = f0_scalar * f0_scalar;
    vec3 F0 = mix(vec3(f0_scalar), uBaseColor, uMetallic);

    // Cook-Torrance BRDF components.
    float D = distributionGGX(N, H, uRoughness);
    float G = geometrySmith(N, V, L, uRoughness);
    vec3  F = fresnelSchlick(max(dot(V, H), 0.0), F0);

    // Specular term.
    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denominator;

    // Energy conservation: diffuse vs specular.
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - uMetallic);

    // Final color.
    vec3 Lo = (kD * uBaseColor / PI + specular) * uLightColor * NdotL;
    vec3 ambient = uAmbient * uBaseColor;

    vec3 color = ambient + Lo + uEmissive;
    // Tone mapping (Reinhard).
    color = color / (color + vec3(1.0));

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

    // Point shader for Scatter3D (circular lit spheres).
    pointInitialized_ = pointShader_.compile(QString::fromLatin1(kPhongVertexShader),
                                              QString::fromLatin1(kPointFragmentShader));

    // PBR shader is optional; we can fall back to Phong if it fails.
    pbrInitialized_ = pbrShader_.compile(QString::fromLatin1(kPbrVertexShader),
                                          QString::fromLatin1(kPbrFragmentShader));

    initialized_ = true;
    return true;
}

ShaderProgram& Renderer3D::shaderForItem(const PlotItem3D& item)
{
    if (item.hasPbrMaterial() && pbrInitialized_)
        return pbrShader_;
    return phongShader_;
}

void Renderer3D::setupPbrUniforms(const PbrMaterial& mat, const RenderContext& ctx)
{
    pbrShader_.setUniform(QStringLiteral("uBaseColor"),
                          QVector3D(mat.baseColor.redF(),
                                    mat.baseColor.greenF(),
                                    mat.baseColor.blueF()));
    pbrShader_.setUniform(QStringLiteral("uMetallic"), mat.metallic);
    pbrShader_.setUniform(QStringLiteral("uRoughness"), mat.roughness);
    pbrShader_.setUniform(QStringLiteral("uIor"), mat.ior);
    pbrShader_.setUniform(QStringLiteral("uEmissive"),
                          QVector3D(mat.emissive.redF(),
                                    mat.emissive.greenF(),
                                    mat.emissive.blueF()));

    Q_UNUSED(ctx);
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

    // Extract light info for shader uniforms.
    QVector3D lightDir(0.3f, -0.8f, -0.5f);
    QVector3D lightColor(1.0f, 0.98f, 0.95f);
    QVector3D ambient(0.35f, 0.35f, 0.38f);

    for (const auto& light : scene.lights()) {
        if (light.type == LightType::Directional) {
            lightDir = light.direction.normalized();
            lightColor = light.color * light.intensity;
        } else if (light.type == LightType::Ambient) {
            ambient = light.color * light.intensity;
        }
    }

    // Set up Phong shader uniforms.
    phongShader_.bind();
    phongShader_.setUniform(QStringLiteral("uLightDir"), lightDir);
    phongShader_.setUniform(QStringLiteral("uLightColor"), lightColor);
    phongShader_.setUniform(QStringLiteral("uAmbient"), ambient);
    phongShader_.setUniform(QStringLiteral("uCameraPos"), camera.position());
    phongShader_.setUniform(QStringLiteral("uSpecularPower"), 32.0f);
    phongShader_.release();

    if (pointInitialized_) {
        pointShader_.bind();
        pointShader_.setUniform(QStringLiteral("uLightDir"), lightDir);
        pointShader_.setUniform(QStringLiteral("uLightColor"), lightColor);
        pointShader_.setUniform(QStringLiteral("uAmbient"), ambient);
        pointShader_.setUniform(QStringLiteral("uCameraPos"), camera.position());
        pointShader_.setUniform(QStringLiteral("uSpecularPower"), 64.0f);
        pointShader_.release();
    }

    // Set up PBR shader uniforms (scene-level).
    if (pbrInitialized_) {
        pbrShader_.bind();
        pbrShader_.setUniform(QStringLiteral("uLightDir"), lightDir);
        pbrShader_.setUniform(QStringLiteral("uLightColor"), lightColor);
        pbrShader_.setUniform(QStringLiteral("uAmbient"), ambient);
        pbrShader_.setUniform(QStringLiteral("uCameraPos"), camera.position());
        pbrShader_.release();
    }

    // Render each visible item with the appropriate shader.
    for (const auto& item : scene.items()) {
        if (item && item->isVisible()) {
            renderItem(*item, ctx);
        }
    }
}

void Renderer3D::renderItem(PlotItem3D& item, const RenderContext& ctx)
{
    ShaderProgram& shader = shaderForItem(item);
    shader.bind();

    if (item.hasPbrMaterial() && pbrInitialized_) {
        setupPbrUniforms(*item.pbrMaterial(), ctx);
    }

    item.render(shader, ctx);
    shader.release();
}

}  // namespace lumen::plot3d
