#pragma once

#include <dashboard/PanelConfig.h>

#include <QDialog>

class QComboBox;
class QLineEdit;
class QSpinBox;

namespace lumen::ui {

class AddPanelDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddPanelDialog(int maxRows, int maxCols,
                             QWidget* parent = nullptr);

    [[nodiscard]] dashboard::PanelConfig result() const;

private:
    QSpinBox* rowSpin_ = nullptr;
    QSpinBox* colSpin_ = nullptr;
    QSpinBox* rowSpanSpin_ = nullptr;
    QSpinBox* colSpanSpin_ = nullptr;
    QLineEdit* titleEdit_ = nullptr;
};

}  // namespace lumen::ui
