#pragma once

#include <plot/Axis.h>

#include <QDialog>
#include <QString>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;

namespace lumen::ui {

/// Dialog for editing axis properties.
///
/// Presents controls for label, range mode, manual range, tick count,
/// tick format, tick format decimals, and grid visibility.  Used by the
/// double-click-to-edit flow in PlotCanvasDock.
class AxisDialog : public QDialog {
    Q_OBJECT

public:
    explicit AxisDialog(QWidget* parent = nullptr);

    /// Populate the dialog controls from current axis properties.
    void setAxisProperties(const QString& label, plot::RangeMode rangeMode,
                           double manualMin, double manualMax,
                           int tickCount, plot::TickFormat tickFormat,
                           int tickFormatDecimals, bool gridVisible);

    [[nodiscard]] QString resultLabel() const;
    [[nodiscard]] plot::RangeMode resultRangeMode() const;
    [[nodiscard]] double resultManualMin() const;
    [[nodiscard]] double resultManualMax() const;
    [[nodiscard]] int resultTickCount() const;
    [[nodiscard]] plot::TickFormat resultTickFormat() const;
    [[nodiscard]] int resultTickFormatDecimals() const;
    [[nodiscard]] bool resultGridVisible() const;
    [[nodiscard]] bool resultLatexMode() const;

private:
    void updateRangeModeVisibility();
    void updateTickFormatVisibility();

    QLineEdit* labelEdit_ = nullptr;
    QComboBox* rangeModeCombo_ = nullptr;
    QDoubleSpinBox* manualMinSpin_ = nullptr;
    QDoubleSpinBox* manualMaxSpin_ = nullptr;
    QSpinBox* tickCountSpin_ = nullptr;
    QComboBox* tickFormatCombo_ = nullptr;
    QSpinBox* tickFormatDecimalsSpin_ = nullptr;
    QCheckBox* gridVisibleCheck_ = nullptr;
    QCheckBox* latexCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
