#include "WorkspaceManager.h"
#include "WorkspaceFile.h"

#include "core/CommandBus.h"
#include "core/DocumentRegistry.h"
#include "core/PlotRegistry.h"

#include <QFileInfo>

namespace lumen::core::io {

WorkspaceManager::WorkspaceManager(DocumentRegistry* docs,
                                   PlotRegistry* plots,
                                   CommandBus* bus,
                                   QObject* parent)
    : QObject(parent)
    , docs_(docs)
    , plots_(plots)
    , bus_(bus)
{
    if (bus_) {
        connect(bus_, &CommandBus::commandExecuted,
                this, &WorkspaceManager::onCommandExecuted);
    }
    if (docs_) {
        connect(docs_, &DocumentRegistry::documentOpened,
                this, &WorkspaceManager::onDocumentOpened);
    }
}

bool WorkspaceManager::saveWorkspace(const QString& docPath)
{
    return saveWorkspaceAs(docPath, defaultSidecarPath(docPath));
}

bool WorkspaceManager::saveWorkspaceAs(const QString& docPath,
                                       const QString& outPath)
{
    auto it = scenes_.find(docPath);
    if (it == scenes_.end() || !it.value())
        return false;

    WorkspaceFile ws = WorkspaceFile::captureFromScene(it.value());
    if (!ws.isValid())
        return false;

    ws.saveToPath(outPath);
    savedPaths_[docPath] = outPath;
    setModified(docPath, false);
    emit workspaceSaved(docPath, outPath);
    return true;
}

bool WorkspaceManager::loadWorkspaceIfExists(const QString& docPath)
{
    const QString sidecar = defaultSidecarPath(docPath);
    if (!QFileInfo::exists(sidecar))
        return false;

    WorkspaceFile ws = WorkspaceFile::loadFromPath(sidecar);
    if (!ws.isValid())
        return false;

    auto it = scenes_.find(docPath);
    if (it == scenes_.end() || !it.value())
        return false;

    const data::DataFrame* df = docs_ ? docs_->document(docPath) : nullptr;
    ws.applyToScene(it.value(), df);

    savedPaths_[docPath] = sidecar;
    setModified(docPath, false);
    emit workspaceLoaded(docPath, sidecar);
    return true;
}

bool WorkspaceManager::revertToSaved(const QString& docPath)
{
    const QString sidecar = savedPaths_.value(docPath, defaultSidecarPath(docPath));
    if (!QFileInfo::exists(sidecar))
        return false;

    WorkspaceFile ws = WorkspaceFile::loadFromPath(sidecar);
    if (!ws.isValid())
        return false;

    auto it = scenes_.find(docPath);
    if (it == scenes_.end() || !it.value())
        return false;

    const data::DataFrame* df = docs_ ? docs_->document(docPath) : nullptr;
    ws.applyToScene(it.value(), df);

    setModified(docPath, false);
    return true;
}

bool WorkspaceManager::isModified(const QString& docPath) const
{
    return modified_.value(docPath, false);
}

QString WorkspaceManager::defaultSidecarPath(const QString& docPath) const
{
    if (docPath.endsWith(QLatin1String(".csv"), Qt::CaseInsensitive)) {
        return docPath.left(docPath.size() - 4) + QLatin1String(".lumen.json");
    }
    return docPath + QLatin1String(".lumen.json");
}

void WorkspaceManager::registerPlotScene(const QString& docPath,
                                         plot::PlotScene* scene)
{
    scenes_[docPath] = scene;
}

void WorkspaceManager::onCommandExecuted(const QString& /*description*/)
{
    // Mark all open documents with registered scenes as modified.
    for (auto it = scenes_.constBegin(); it != scenes_.constEnd(); ++it) {
        if (it.value())
            setModified(it.key(), true);
    }
}

void WorkspaceManager::onDocumentOpened(const QString& docPath)
{
    loadWorkspaceIfExists(docPath);
}

void WorkspaceManager::setModified(const QString& docPath, bool modified)
{
    const bool was = modified_.value(docPath, false);
    modified_[docPath] = modified;
    if (was != modified)
        emit modifiedChanged(docPath, modified);
}

}  // namespace lumen::core::io
