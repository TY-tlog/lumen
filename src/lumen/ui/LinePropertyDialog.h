#pragma once

#include <plot/PlotStyle.h>

#include <QDialog>
#include <QString>

class QCheckBox;
class QColorDialog;
class QComboBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;

namespace lumen::ui {

/// Dialog for editing line series visual properties.
///
/// Presents controls for color, line width, pen style, series name,
/// and visibility.  Used by the double-click-to-edit flow in
/// PlotCanvasDock.
class LinePropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit LinePropertyDialog(QWidget* parent = nullptr);

    /// Populate the dialog controls from current series properties.
    void setStyle(const plot::PlotStyle& style, const QString& name, bool visible);

    /// Retrieve the edited style after the dialog is accepted.
    [[nodiscard]] plot::PlotStyle resultStyle() const;

    /// Retrieve the edited series name after the dialog is accepted.
    [[nodiscard]] QString resultName() const;

    /// Retrieve the edited visibility after the dialog is accepted.
    [[nodiscard]] bool resultVisible() const;

private:
    void updateColorButton();

    QColor currentColor_;
    QPushButton* colorButton_ = nullptr;
    QDoubleSpinBox* widthSpin_ = nullptr;
    QComboBox* styleCombo_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QCheckBox* visibleCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
