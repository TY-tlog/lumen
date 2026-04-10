#pragma once

#include <plot/ScatterSeries.h>

#include <QDialog>
#include <QString>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

namespace lumen::ui {

/// Dialog for editing scatter series visual properties.
class ScatterPropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScatterPropertyDialog(QWidget* parent = nullptr);

    void setProperties(QColor color, plot::MarkerShape shape, int size,
                       bool filled, const QString& name, bool visible);

    [[nodiscard]] QColor resultColor() const { return currentColor_; }
    [[nodiscard]] plot::MarkerShape resultMarkerShape() const;
    [[nodiscard]] int resultMarkerSize() const;
    [[nodiscard]] bool resultFilled() const;
    [[nodiscard]] QString resultName() const;
    [[nodiscard]] bool resultVisible() const;

private:
    void updateColorButton();

    QColor currentColor_;
    QPushButton* colorButton_ = nullptr;
    QComboBox* shapeCombo_ = nullptr;
    QSpinBox* sizeSpin_ = nullptr;
    QCheckBox* filledCheck_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QCheckBox* visibleCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
