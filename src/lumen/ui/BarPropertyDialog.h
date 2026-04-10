#pragma once

#include <QDialog>
#include <QColor>
#include <QString>

class QCheckBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;

namespace lumen::ui {

/// Dialog for editing bar series visual properties.
class BarPropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit BarPropertyDialog(QWidget* parent = nullptr);

    void setProperties(QColor fillColor, QColor outlineColor, double barWidth,
                       const QString& name, bool visible);

    [[nodiscard]] QColor resultFillColor() const { return currentFillColor_; }
    [[nodiscard]] QColor resultOutlineColor() const;
    [[nodiscard]] double resultBarWidth() const;
    [[nodiscard]] QString resultName() const;
    [[nodiscard]] bool resultVisible() const;

private:
    void updateFillColorButton();
    void updateOutlineColorButton();

    QColor currentFillColor_;
    QColor currentOutlineColor_;
    QPushButton* fillColorButton_ = nullptr;
    QPushButton* outlineColorButton_ = nullptr;
    QCheckBox* outlineNoneCheck_ = nullptr;
    QDoubleSpinBox* barWidthSpin_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QCheckBox* visibleCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
