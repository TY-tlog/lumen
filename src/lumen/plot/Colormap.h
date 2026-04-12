#pragma once

#include <QColor>
#include <QJsonObject>
#include <QString>

#include <optional>
#include <utility>
#include <vector>

namespace lumen::plot {

/// A colormap defined by a set of control points (t, QColor).
/// sample(t) linearly interpolates between the two bracketing stops.
/// Provides perceptual-uniformity (CIELAB) and colorblind-safety
/// (Machado 2009) diagnostics per ADR-040.
class Colormap {
public:
    enum class Builtin {
        Viridis, Plasma, Inferno, Magma, Turbo,
        Cividis,
        Gray, Hot, Cool,
        RedBlue, BrownTeal
    };

    /// Create a built-in colormap by ID.
    static Colormap builtin(Builtin id);

    /// Create from explicit control points. Stops must be sorted by t.
    static Colormap fromControlPoints(
        const std::vector<std::pair<double, QColor>>& stops,
        const QString& name = {});

    /// Deserialise from JSON ({"name": "...", "stops": [[t,r,g,b], ...]}).
    static Colormap fromJson(const QJsonObject& obj);

    /// Linearly interpolate the colormap at parameter t (clamped to [0,1]).
    [[nodiscard]] QColor sample(double t) const;

    /// ADR-040: CIELAB delta-E coefficient of variation < 0.4.
    [[nodiscard]] bool isPerceptuallyUniform() const;

    /// ADR-040: Machado 2009 CVD simulation — min delta-E > 2.0
    /// for deuteranopia and protanopia at severity 1.0.
    [[nodiscard]] bool isColorblindSafe() const;

    [[nodiscard]] QString name() const { return name_; }

    /// Serialise to JSON.
    [[nodiscard]] QJsonObject toJson() const;

    /// Access the internal stops (sorted by t).
    [[nodiscard]] const std::vector<std::pair<double, QColor>>& stops() const
    { return stops_; }

private:
    Colormap() = default;

    QString name_;
    std::vector<std::pair<double, QColor>> stops_; // sorted by t
    mutable std::optional<bool> uniformCache_;
    mutable std::optional<bool> cvdSafeCache_;
};

} // namespace lumen::plot
