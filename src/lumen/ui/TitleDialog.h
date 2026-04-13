#pragma once

#include <QDialog>
#include <QFont>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QSpinBox;

namespace lumen::ui {

/// Dialog for editing plot title font properties (right-click flow).
///
/// The title text itself is edited inline on the canvas. This dialog
/// controls font size and weight only.
class TitleDialog : public QDialog {
    Q_OBJECT

public:
    explicit TitleDialog(QWidget* parent = nullptr);

    /// Populate the dialog controls from current title properties.
    void setTitleProperties(int fontPx, QFont::Weight weight);

    [[nodiscard]] int resultFontPx() const;
    [[nodiscard]] QFont::Weight resultWeight() const;
    [[nodiscard]] bool resultLatexMode() const;

private:
    QSpinBox* fontPxSpin_ = nullptr;
    QComboBox* weightCombo_ = nullptr;
    QCheckBox* latexCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
