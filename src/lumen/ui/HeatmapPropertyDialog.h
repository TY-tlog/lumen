#pragma once

#include <plot/Colormap.h>
#include <plot/Heatmap.h>

#include <QDialog>
#include <QString>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLineEdit;
class QSlider;

namespace lumen::ui {

/// Dialog for editing heatmap visual properties.
///
/// Presents controls for colormap, value range, interpolation, opacity,
/// name, and visibility.
class HeatmapPropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit HeatmapPropertyDialog(QWidget* parent = nullptr);

    /// Populate dialog controls from current heatmap properties.
    void setProperties(const plot::Colormap& colormap, double valueMin,
                       double valueMax, bool autoRange,
                       plot::Heatmap::Interpolation interpolation,
                       double opacity, const QString& name, bool visible);

    [[nodiscard]] plot::Colormap resultColormap() const;
    [[nodiscard]] double resultValueMin() const;
    [[nodiscard]] double resultValueMax() const;
    [[nodiscard]] bool resultAutoRange() const;
    [[nodiscard]] plot::Heatmap::Interpolation resultInterpolation() const;
    [[nodiscard]] double resultOpacity() const;
    [[nodiscard]] QString resultName() const;
    [[nodiscard]] bool resultVisible() const;

private:
    QComboBox* colormapCombo_ = nullptr;
    QCheckBox* autoRangeCheck_ = nullptr;
    QDoubleSpinBox* valueMinSpin_ = nullptr;
    QDoubleSpinBox* valueMaxSpin_ = nullptr;
    QComboBox* interpCombo_ = nullptr;
    QSlider* opacitySlider_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QCheckBox* visibleCheck_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
