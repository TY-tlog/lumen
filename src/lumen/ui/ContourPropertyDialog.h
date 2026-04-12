#pragma once

#include <QColor>
#include <QDialog>
#include <QString>

#include <vector>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;

namespace lumen::ui {

/// Dialog for editing contour plot visual properties.
///
/// Presents controls for level count (auto), manual levels, line color,
/// line width, labels visibility, name, and visibility.
class ContourPropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit ContourPropertyDialog(QWidget* parent = nullptr);

    /// Populate dialog controls from current contour properties.
    void setProperties(const std::vector<double>& levels, int autoLevelCount,
                       QColor lineColor, double lineWidth, bool labelsVisible,
                       const QString& name, bool visible);

    [[nodiscard]] std::vector<double> resultLevels() const;
    [[nodiscard]] int resultAutoLevelCount() const;
    [[nodiscard]] QColor resultLineColor() const { return currentLineColor_; }
    [[nodiscard]] double resultLineWidth() const;
    [[nodiscard]] bool resultLabelsVisible() const;
    [[nodiscard]] QString resultName() const;
    [[nodiscard]] bool resultVisible() const;

private:
    void updateColorButton();

    QCheckBox* autoLevelsCheck_ = nullptr;
    QSpinBox* levelCountSpin_ = nullptr;
    QLineEdit* manualLevelsEdit_ = nullptr;
    QColor currentLineColor_;
    QPushButton* lineColorButton_ = nullptr;
    QDoubleSpinBox* lineWidthSpin_ = nullptr;
    QCheckBox* labelsCheck_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QCheckBox* visibleCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
