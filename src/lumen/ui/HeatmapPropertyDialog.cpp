#include "HeatmapPropertyDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSlider>

namespace lumen::ui {

namespace {

struct ColormapEntry {
    const char* label;
    plot::Colormap::Builtin id;
};

constexpr ColormapEntry kColormaps[] = {
    {"Viridis",   plot::Colormap::Builtin::Viridis},
    {"Plasma",    plot::Colormap::Builtin::Plasma},
    {"Inferno",   plot::Colormap::Builtin::Inferno},
    {"Magma",     plot::Colormap::Builtin::Magma},
    {"Turbo",     plot::Colormap::Builtin::Turbo},
    {"Cividis",   plot::Colormap::Builtin::Cividis},
    {"Gray",      plot::Colormap::Builtin::Gray},
    {"Hot",       plot::Colormap::Builtin::Hot},
    {"Cool",      plot::Colormap::Builtin::Cool},
    {"RedBlue",   plot::Colormap::Builtin::RedBlue},
    {"BrownTeal", plot::Colormap::Builtin::BrownTeal},
};

}  // namespace

HeatmapPropertyDialog::HeatmapPropertyDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Heatmap Properties"));

    auto* layout = new QFormLayout(this);

    // Colormap picker.
    colormapCombo_ = new QComboBox(this);
    for (const auto& entry : kColormaps) {
        auto cmap = plot::Colormap::builtin(entry.id);
        QString label = QString::fromLatin1(entry.label);
        if (cmap.isPerceptuallyUniform()) {
            label += QStringLiteral(" \u2713");  // checkmark badge
        }
        colormapCombo_->addItem(label, static_cast<int>(entry.id));
    }
    layout->addRow(tr("Colormap:"), colormapCombo_);

    // Value range.
    autoRangeCheck_ = new QCheckBox(tr("Auto range"), this);
    layout->addRow(QString(), autoRangeCheck_);

    valueMinSpin_ = new QDoubleSpinBox(this);
    valueMinSpin_->setRange(-1e12, 1e12);
    valueMinSpin_->setDecimals(4);
    layout->addRow(tr("Value min:"), valueMinSpin_);

    valueMaxSpin_ = new QDoubleSpinBox(this);
    valueMaxSpin_->setRange(-1e12, 1e12);
    valueMaxSpin_->setDecimals(4);
    layout->addRow(tr("Value max:"), valueMaxSpin_);

    connect(autoRangeCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        valueMinSpin_->setEnabled(!checked);
        valueMaxSpin_->setEnabled(!checked);
    });

    // Interpolation.
    interpCombo_ = new QComboBox(this);
    interpCombo_->addItem(tr("Nearest"),
        static_cast<int>(plot::Heatmap::Interpolation::Nearest));
    interpCombo_->addItem(tr("Bilinear"),
        static_cast<int>(plot::Heatmap::Interpolation::Bilinear));
    layout->addRow(tr("Interpolation:"), interpCombo_);

    // Opacity slider (0-100 mapped to 0.0-1.0).
    opacitySlider_ = new QSlider(Qt::Horizontal, this);
    opacitySlider_->setRange(0, 100);
    opacitySlider_->setValue(100);
    layout->addRow(tr("Opacity:"), opacitySlider_);

    // Name.
    nameEdit_ = new QLineEdit(this);
    layout->addRow(tr("Name:"), nameEdit_);

    // Visible.
    visibleCheck_ = new QCheckBox(tr("Visible"), this);
    layout->addRow(QString(), visibleCheck_);

    // OK / Cancel.
    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttonBox_);
}

void HeatmapPropertyDialog::setProperties(
    const plot::Colormap& colormap, double valueMin, double valueMax,
    bool autoRange, plot::Heatmap::Interpolation interpolation,
    double opacity, const QString& name, bool visible) {
    // Find colormap by name.
    for (int i = 0; i < colormapCombo_->count(); ++i) {
        auto id = static_cast<plot::Colormap::Builtin>(
            colormapCombo_->itemData(i).toInt());
        auto cmap = plot::Colormap::builtin(id);
        if (cmap.name() == colormap.name()) {
            colormapCombo_->setCurrentIndex(i);
            break;
        }
    }

    autoRangeCheck_->setChecked(autoRange);
    valueMinSpin_->setValue(valueMin);
    valueMaxSpin_->setValue(valueMax);
    valueMinSpin_->setEnabled(!autoRange);
    valueMaxSpin_->setEnabled(!autoRange);

    int interpIdx = interpCombo_->findData(static_cast<int>(interpolation));
    if (interpIdx >= 0) {
        interpCombo_->setCurrentIndex(interpIdx);
    }

    opacitySlider_->setValue(static_cast<int>(opacity * 100.0));
    nameEdit_->setText(name);
    visibleCheck_->setChecked(visible);
}

plot::Colormap HeatmapPropertyDialog::resultColormap() const {
    auto id = static_cast<plot::Colormap::Builtin>(
        colormapCombo_->currentData().toInt());
    return plot::Colormap::builtin(id);
}

double HeatmapPropertyDialog::resultValueMin() const {
    return valueMinSpin_->value();
}

double HeatmapPropertyDialog::resultValueMax() const {
    return valueMaxSpin_->value();
}

bool HeatmapPropertyDialog::resultAutoRange() const {
    return autoRangeCheck_->isChecked();
}

plot::Heatmap::Interpolation HeatmapPropertyDialog::resultInterpolation() const {
    return static_cast<plot::Heatmap::Interpolation>(
        interpCombo_->currentData().toInt());
}

double HeatmapPropertyDialog::resultOpacity() const {
    return opacitySlider_->value() / 100.0;
}

QString HeatmapPropertyDialog::resultName() const {
    return nameEdit_->text();
}

bool HeatmapPropertyDialog::resultVisible() const {
    return visibleCheck_->isChecked();
}

}  // namespace lumen::ui
