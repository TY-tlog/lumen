#pragma once

#include "style/cascade.h"

#include <QDialog>

class QButtonGroup;
class QDialogButtonBox;
class QLabel;
class QRadioButton;

namespace lumen::ui {

/// Dialog for choosing the cascade level when editing a style property.
///
/// Three options (D10.5):
/// 1. This element only (priority 4, default)
/// 2. All matching in this plot (priority 3)
/// 3. Save to current theme (priority 1, with confirmation)
class PromotionDialog : public QDialog {
    Q_OBJECT

public:
    explicit PromotionDialog(const QString& elementName,
                              const QString& themeName,
                              bool themeIsBuiltin,
                              QWidget* parent = nullptr);

    /// The cascade level the user selected.
    [[nodiscard]] style::CascadeLevel selectedLevel() const;

    /// Whether the user chose to fork the built-in theme.
    [[nodiscard]] bool wantsFork() const { return wantsFork_; }

    /// The new theme name if forking.
    [[nodiscard]] QString forkName() const { return forkName_; }

private:
    void onThemeRadioToggled(bool checked);

    QRadioButton* elementRadio_ = nullptr;
    QRadioButton* plotRadio_ = nullptr;
    QRadioButton* themeRadio_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
    bool themeIsBuiltin_ = false;
    bool wantsFork_ = false;
    QString forkName_;
    QString themeName_;
};

}  // namespace lumen::ui
