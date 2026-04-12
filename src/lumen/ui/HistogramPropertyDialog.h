#pragma once

#include <plot/HistogramSeries.h>

#include <QColor>
#include <QDialog>
#include <QString>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

namespace lumen::ui {

/// Dialog for editing histogram series visual properties.
///
/// Presents controls for bin count, bin rule, normalization, fill color,
/// name, and visibility.
class HistogramPropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit HistogramPropertyDialog(QWidget* parent = nullptr);

    /// Populate dialog controls from current histogram properties.
    void setProperties(int binCount, plot::HistogramSeries::BinRule binRule,
                       plot::HistogramSeries::Normalization normalization,
                       QColor fillColor, const QString& name, bool visible);

    [[nodiscard]] int resultBinCount() const;
    [[nodiscard]] plot::HistogramSeries::BinRule resultBinRule() const;
    [[nodiscard]] plot::HistogramSeries::Normalization resultNormalization() const;
    [[nodiscard]] QColor resultFillColor() const { return currentFillColor_; }
    [[nodiscard]] QString resultName() const;
    [[nodiscard]] bool resultVisible() const;

private:
    void updateFillColorButton();

    QCheckBox* autoBinCheck_ = nullptr;
    QSpinBox* binCountSpin_ = nullptr;
    QComboBox* binRuleCombo_ = nullptr;
    QComboBox* normCombo_ = nullptr;
    QColor currentFillColor_;
    QPushButton* fillColorButton_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QCheckBox* visibleCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
