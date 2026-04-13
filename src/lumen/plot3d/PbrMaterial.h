#pragma once

#include <QColor>
#include <QJsonObject>

namespace lumen::plot3d {

/// PBR (Physically Based Rendering) material parameters.
///
/// Used with the Cook-Torrance BRDF shader for realistic material appearance.
struct PbrMaterial {
    QColor baseColor{200, 200, 200};
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ior = 1.5f;
    QColor emissive{0, 0, 0};

    [[nodiscard]] QJsonObject toJson() const
    {
        QJsonObject obj;
        obj[QStringLiteral("baseColor_r")] = baseColor.red();
        obj[QStringLiteral("baseColor_g")] = baseColor.green();
        obj[QStringLiteral("baseColor_b")] = baseColor.blue();
        obj[QStringLiteral("metallic")] = static_cast<double>(metallic);
        obj[QStringLiteral("roughness")] = static_cast<double>(roughness);
        obj[QStringLiteral("ior")] = static_cast<double>(ior);
        obj[QStringLiteral("emissive_r")] = emissive.red();
        obj[QStringLiteral("emissive_g")] = emissive.green();
        obj[QStringLiteral("emissive_b")] = emissive.blue();
        return obj;
    }

    static PbrMaterial fromJson(const QJsonObject& obj)
    {
        PbrMaterial mat;
        mat.baseColor = QColor(
            obj[QStringLiteral("baseColor_r")].toInt(200),
            obj[QStringLiteral("baseColor_g")].toInt(200),
            obj[QStringLiteral("baseColor_b")].toInt(200));
        mat.metallic = static_cast<float>(obj[QStringLiteral("metallic")].toDouble(0.0));
        mat.roughness = static_cast<float>(obj[QStringLiteral("roughness")].toDouble(0.5));
        mat.ior = static_cast<float>(obj[QStringLiteral("ior")].toDouble(1.5));
        mat.emissive = QColor(
            obj[QStringLiteral("emissive_r")].toInt(0),
            obj[QStringLiteral("emissive_g")].toInt(0),
            obj[QStringLiteral("emissive_b")].toInt(0));
        return mat;
    }

    bool operator==(const PbrMaterial& other) const
    {
        return baseColor == other.baseColor &&
               metallic == other.metallic &&
               roughness == other.roughness &&
               ior == other.ior &&
               emissive == other.emissive;
    }

    bool operator!=(const PbrMaterial& other) const
    {
        return !(*this == other);
    }
};

}  // namespace lumen::plot3d
