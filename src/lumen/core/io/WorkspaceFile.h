#pragma once

#include <QJsonObject>
#include <QString>

namespace lumen::data {
class DataFrame;
}

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::io {

/// Serializes/deserializes a .lumen.json workspace sidecar file.
///
/// Captures the full visual state of a PlotScene (viewport, title,
/// axes, legend, series styles) as JSON and can restore it later.
/// See ADR-025 for the schema definition.
class WorkspaceFile {
public:
    /// Load from a .lumen.json file.  Returns an invalid WorkspaceFile
    /// if the file doesn't exist or JSON is malformed.
    static WorkspaceFile loadFromPath(const QString& path);

    /// Save current state to a .lumen.json file.
    void saveToPath(const QString& path) const;

    /// Capture all serializable state from a PlotScene.
    static WorkspaceFile captureFromScene(const plot::PlotScene* scene);

    /// Apply saved state to a PlotScene.  Needs the DataFrame to
    /// resolve column names to Column pointers for series.
    void applyToScene(plot::PlotScene* scene,
                      const data::DataFrame* df) const;

    /// Whether this workspace file was loaded/created successfully.
    [[nodiscard]] bool isValid() const;

    /// Schema version (currently always 1).
    [[nodiscard]] int version() const;

    /// Access the underlying JSON data (for testing).
    [[nodiscard]] const QJsonObject& data() const { return data_; }

private:
    QJsonObject data_;
    bool valid_ = false;
};

}  // namespace lumen::core::io
