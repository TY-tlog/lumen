#pragma once

#include <plot/BoxPlotSeries.h>

#include <QColor>
#include <QDialog>
#include <QString>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;

namespace lumen::ui {

/// Dialog for editing box plot series visual properties.
///
/// Presents controls for whisker rule, notched, outliers visible, fill color,
/// name, and visibility.
class BoxPlotPropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit BoxPlotPropertyDialog(QWidget* parent = nullptr);

    /// Populate dialog controls from current box plot properties.
    void setProperties(plot::BoxPlotSeries::WhiskerRule whiskerRule,
                       bool notched, bool outliersVisible, QColor fillColor,
                       const QString& name, bool visible);

    [[nodiscard]] plot::BoxPlotSeries::WhiskerRule resultWhiskerRule() const;
    [[nodiscard]] bool resultNotched() const;
    [[nodiscard]] bool resultOutliersVisible() const;
    [[nodiscard]] QColor resultFillColor() const { return currentFillColor_; }
    [[nodiscard]] QString resultName() const;
    [[nodiscard]] bool resultVisible() const;

private:
    void updateFillColorButton();

    QComboBox* whiskerRuleCombo_ = nullptr;
    QCheckBox* notchedCheck_ = nullptr;
    QCheckBox* outliersCheck_ = nullptr;
    QColor currentFillColor_;
    QPushButton* fillColorButton_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QCheckBox* visibleCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
