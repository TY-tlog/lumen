#pragma once

#include <core/Command.h>
#include <dashboard/PanelConfig.h>

#include <QString>

namespace lumen::dashboard {
class Dashboard;
}

namespace lumen::core::commands {

class AddDashboardPanelCommand : public Command {
public:
    AddDashboardPanelCommand(dashboard::Dashboard* db,
                              dashboard::PanelConfig config);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    dashboard::Dashboard* db_;
    dashboard::PanelConfig config_;
    int addedIndex_ = -1;
};

class RemoveDashboardPanelCommand : public Command {
public:
    RemoveDashboardPanelCommand(dashboard::Dashboard* db, int index);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    dashboard::Dashboard* db_;
    int index_;
    dashboard::PanelConfig savedConfig_;
};

class ChangeDashboardLayoutCommand : public Command {
public:
    ChangeDashboardLayoutCommand(dashboard::Dashboard* db,
                                  int newRows, int newCols);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    dashboard::Dashboard* db_;
    int newRows_, newCols_;
    int oldRows_, oldCols_;
};

}  // namespace lumen::core::commands
