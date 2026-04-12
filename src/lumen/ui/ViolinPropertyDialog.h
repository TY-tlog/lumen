#pragma once

#include <QColor>
#include <QDialog>
#include <QString>

class QCheckBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;

namespace lumen::ui {

/// Dialog for editing violin series visual properties.
///
/// Presents controls for KDE bandwidth, auto KDE, split, fill color,
/// name, and visibility.
class ViolinPropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit ViolinPropertyDialog(QWidget* parent = nullptr);

    /// Populate dialog controls from current violin properties.
    void setProperties(double bandwidth, bool autoKde, bool split,
                       QColor fillColor, const QString& name, bool visible);

    [[nodiscard]] double resultBandwidth() const;
    [[nodiscard]] bool resultAutoKde() const;
    [[nodiscard]] bool resultSplit() const;
    [[nodiscard]] QColor resultFillColor() const { return currentFillColor_; }
    [[nodiscard]] QString resultName() const;
    [[nodiscard]] bool resultVisible() const;

private:
    void updateFillColorButton();

    QCheckBox* autoKdeCheck_ = nullptr;
    QDoubleSpinBox* bandwidthSpin_ = nullptr;
    QCheckBox* splitCheck_ = nullptr;
    QColor currentFillColor_;
    QPushButton* fillColorButton_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QCheckBox* visibleCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
