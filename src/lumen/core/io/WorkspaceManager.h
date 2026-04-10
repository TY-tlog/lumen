#pragma once

#include <QHash>
#include <QObject>
#include <QString>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core {
class CommandBus;
class DocumentRegistry;
class PlotRegistry;
}

namespace lumen::core::io {

/// Manages workspace sidecar files (.lumen.json) for open documents.
///
/// Tracks modification state, auto-loads workspace on document open,
/// and provides save/revert operations.  Connects to CommandBus to
/// detect edits and to DocumentRegistry to detect new documents.
class WorkspaceManager : public QObject {
    Q_OBJECT

public:
    explicit WorkspaceManager(DocumentRegistry* docs,
                              PlotRegistry* plots,
                              CommandBus* bus,
                              QObject* parent = nullptr);

    /// Save the workspace for @p docPath to its default sidecar path.
    /// Returns true on success.
    bool saveWorkspace(const QString& docPath);

    /// Save the workspace for @p docPath to a custom @p outPath.
    /// Returns true on success.
    bool saveWorkspaceAs(const QString& docPath, const QString& outPath);

    /// If a sidecar file exists for @p docPath, load and apply it.
    /// Returns true if a sidecar was found and applied.
    bool loadWorkspaceIfExists(const QString& docPath);

    /// Revert the plot scene for @p docPath to the last saved state.
    /// Returns true if a sidecar was found and applied.
    bool revertToSaved(const QString& docPath);

    /// Whether the document at @p docPath has unsaved modifications.
    [[nodiscard]] bool isModified(const QString& docPath) const;

    /// Compute the default sidecar path: replace .csv → .lumen.json.
    /// If no .csv extension, append .lumen.json.
    [[nodiscard]] QString defaultSidecarPath(const QString& docPath) const;

    /// Register a PlotScene for a document path.
    /// Called by PlotCanvasDock (or equivalent) when setting up a canvas.
    void registerPlotScene(const QString& docPath, plot::PlotScene* scene);

signals:
    void modifiedChanged(const QString& docPath, bool modified);
    void workspaceSaved(const QString& docPath, const QString& outPath);
    void workspaceLoaded(const QString& docPath, const QString& inPath);

private:
    void onCommandExecuted(const QString& description);
    void onDocumentOpened(const QString& docPath);
    void setModified(const QString& docPath, bool modified);

    DocumentRegistry* docs_;
    PlotRegistry* plots_;
    CommandBus* bus_;
    QHash<QString, bool> modified_;
    QHash<QString, QString> savedPaths_;
    QHash<QString, plot::PlotScene*> scenes_;
};

}  // namespace lumen::core::io
