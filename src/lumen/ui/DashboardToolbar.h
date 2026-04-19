#pragma once

#include <QToolBar>

class QComboBox;
class QPushButton;

namespace lumen::dashboard {
class Dashboard;
}

namespace lumen::ui {

class DashboardToolbar : public QToolBar {
    Q_OBJECT

public:
    explicit DashboardToolbar(dashboard::Dashboard* dashboard,
                               QWidget* parent = nullptr);

signals:
    void addPanelRequested();
    void linkEditorRequested();

private slots:
    void onGridPresetChanged(int index);

private:
    dashboard::Dashboard* dashboard_;
    QComboBox* gridCombo_ = nullptr;
};

}  // namespace lumen::ui
