#include "DashboardCommands.h"

#include <dashboard/Dashboard.h>

namespace lumen::core::commands {

// --- AddDashboardPanelCommand ---

AddDashboardPanelCommand::AddDashboardPanelCommand(
    dashboard::Dashboard* db, dashboard::PanelConfig config)
    : db_(db)
    , config_(std::move(config))
{
}

void AddDashboardPanelCommand::execute()
{
    addedIndex_ = db_->addPanel(config_);
}

void AddDashboardPanelCommand::undo()
{
    if (addedIndex_ >= 0)
        db_->removePanel(addedIndex_);
}

QString AddDashboardPanelCommand::description() const
{
    return QStringLiteral("Add dashboard panel");
}

// --- RemoveDashboardPanelCommand ---

RemoveDashboardPanelCommand::RemoveDashboardPanelCommand(
    dashboard::Dashboard* db, int index)
    : db_(db)
    , index_(index)
{
}

void RemoveDashboardPanelCommand::execute()
{
    if (index_ >= 0 && index_ < db_->panelCount())
        savedConfig_ = db_->panelConfigAt(index_);
    db_->removePanel(index_);
}

void RemoveDashboardPanelCommand::undo()
{
    db_->addPanel(savedConfig_);
}

QString RemoveDashboardPanelCommand::description() const
{
    return QStringLiteral("Remove dashboard panel");
}

// --- ChangeDashboardLayoutCommand ---

ChangeDashboardLayoutCommand::ChangeDashboardLayoutCommand(
    dashboard::Dashboard* db, int newRows, int newCols)
    : db_(db)
    , newRows_(newRows)
    , newCols_(newCols)
    , oldRows_(db->gridRows())
    , oldCols_(db->gridCols())
{
}

void ChangeDashboardLayoutCommand::execute()
{
    db_->setGridSize(newRows_, newCols_);
}

void ChangeDashboardLayoutCommand::undo()
{
    db_->setGridSize(oldRows_, oldCols_);
}

QString ChangeDashboardLayoutCommand::description() const
{
    return QStringLiteral("Change dashboard layout");
}

}  // namespace lumen::core::commands
