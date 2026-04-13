#include "ExportDialog.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

namespace lumen::ui {

ExportDialog::ExportDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Export Figure"));
    setMinimumWidth(420);

    auto* layout = new QFormLayout(this);

    // --- Format radio buttons ---
    formatGroup_ = new QButtonGroup(this);
    pngRadio_ = new QRadioButton(tr("PNG"), this);
    svgRadio_ = new QRadioButton(tr("SVG"), this);
    pdfRadio_ = new QRadioButton(tr("PDF"), this);
    formatGroup_->addButton(pngRadio_, 0);
    formatGroup_->addButton(svgRadio_, 1);
    formatGroup_->addButton(pdfRadio_, 2);
    pngRadio_->setChecked(true);

    auto* formatLayout = new QHBoxLayout;
    formatLayout->addWidget(pngRadio_);
    formatLayout->addWidget(svgRadio_);
    formatLayout->addWidget(pdfRadio_);
    formatLayout->addStretch();
    layout->addRow(tr("Format:"), formatLayout);

    connect(formatGroup_, &QButtonGroup::idClicked,
            this, [this](int /*id*/) {
        updateControlVisibility();
        updatePathExtension();
    });

    // --- Size preset ---
    sizePreset_ = new QComboBox(this);
    sizePreset_->addItem(tr("Publication single column (1050x700)"), 0);
    sizePreset_->addItem(tr("Publication double column (2100x1400)"), 1);
    sizePreset_->addItem(tr("Custom"), 2);
    layout->addRow(tr("Size:"), sizePreset_);

    connect(sizePreset_, &QComboBox::currentIndexChanged,
            this, [this](int /*index*/) { updateControlVisibility(); });

    // --- Custom width / height ---
    widthLabel_ = new QLabel(tr("Width:"), this);
    widthSpin_ = new QSpinBox(this);
    widthSpin_->setRange(50, 10000);
    widthSpin_->setValue(1050);
    widthSpin_->setSuffix(QStringLiteral(" px"));
    layout->addRow(widthLabel_, widthSpin_);

    heightLabel_ = new QLabel(tr("Height:"), this);
    heightSpin_ = new QSpinBox(this);
    heightSpin_->setRange(50, 10000);
    heightSpin_->setValue(700);
    heightSpin_->setSuffix(QStringLiteral(" px"));
    layout->addRow(heightLabel_, heightSpin_);

    // --- DPI ---
    dpiCombo_ = new QComboBox(this);
    dpiCombo_->addItem(QStringLiteral("72"), 72);
    dpiCombo_->addItem(QStringLiteral("150"), 150);
    dpiCombo_->addItem(QStringLiteral("300"), 300);
    dpiCombo_->addItem(QStringLiteral("600"), 600);
    dpiCombo_->setCurrentIndex(2);  // default 300
    layout->addRow(tr("DPI:"), dpiCombo_);

    // --- Background ---
    bgCombo_ = new QComboBox(this);
    bgCombo_->addItem(tr("White"), false);
    bgCombo_->addItem(tr("Transparent"), true);
    layout->addRow(tr("Background:"), bgCombo_);

    // --- Color profile (Phase 9 ICC) ---
    profileCombo_ = new QComboBox(this);
    profileCombo_->addItem(tr("sRGB (screen default)"), 0);
    profileCombo_->addItem(tr("Adobe RGB (1998)"), 1);
    profileCombo_->addItem(tr("Display P3"), 2);
    layout->addRow(tr("Color profile:"), profileCombo_);

    // --- Output path ---
    pathEdit_ = new QLineEdit(this);
    pathEdit_->setPlaceholderText(tr("Choose output file..."));
    browseBtn_ = new QPushButton(tr("Browse..."), this);
    auto* pathLayout = new QHBoxLayout;
    pathLayout->addWidget(pathEdit_, 1);
    pathLayout->addWidget(browseBtn_);
    layout->addRow(tr("Output:"), pathLayout);

    connect(browseBtn_, &QPushButton::clicked, this, &ExportDialog::onBrowse);

    // --- Export / Cancel ---
    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox_->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttonBox_);

    // Initial visibility state.
    updateControlVisibility();
}

ExportDialog::ExportOptions ExportDialog::options() const {
    ExportOptions opts;
    opts.format = formatGroup_->checkedId();
    opts.dpi = dpiCombo_->currentData().toInt();
    opts.transparentBackground = bgCombo_->currentData().toBool();
    opts.outputPath = pathEdit_->text();
    opts.colorProfileIndex = profileCombo_->currentData().toInt();

    int presetData = sizePreset_->currentData().toInt();
    switch (presetData) {
        case 0:  // single column
            opts.widthPx = 1050;
            opts.heightPx = 700;
            break;
        case 1:  // double column
            opts.widthPx = 2100;
            opts.heightPx = 1400;
            break;
        case 2:  // custom
            opts.widthPx = widthSpin_->value();
            opts.heightPx = heightSpin_->value();
            break;
    }

    return opts;
}

void ExportDialog::updateControlVisibility() {
    // Custom size controls visible only when "Custom" preset selected.
    bool customSize = (sizePreset_->currentData().toInt() == 2);
    widthSpin_->setVisible(customSize);
    heightSpin_->setVisible(customSize);
    widthLabel_->setVisible(customSize);
    heightLabel_->setVisible(customSize);

    // SVG ignores DPI and background.
    bool isSvg = (formatGroup_->checkedId() == 1);
    dpiCombo_->setEnabled(!isSvg);
    bgCombo_->setEnabled(!isSvg);
}

void ExportDialog::updatePathExtension() {
    QString path = pathEdit_->text();
    if (path.isEmpty()) {
        return;
    }

    // Strip existing known extension.
    static const QStringList knownExts = {
        QStringLiteral(".png"),
        QStringLiteral(".svg"),
        QStringLiteral(".pdf"),
    };
    for (const auto& ext : knownExts) {
        if (path.endsWith(ext, Qt::CaseInsensitive)) {
            path.chop(ext.size());
            break;
        }
    }

    // Append new extension based on selected format.
    switch (formatGroup_->checkedId()) {
        case 0: path += QStringLiteral(".png"); break;
        case 1: path += QStringLiteral(".svg"); break;
        case 2: path += QStringLiteral(".pdf"); break;
    }
    pathEdit_->setText(path);
}

void ExportDialog::onBrowse() {
    QString filter;
    QString defaultSuffix;
    switch (formatGroup_->checkedId()) {
        case 0:
            filter = tr("PNG Images (*.png)");
            defaultSuffix = QStringLiteral("png");
            break;
        case 1:
            filter = tr("SVG Files (*.svg)");
            defaultSuffix = QStringLiteral("svg");
            break;
        case 2:
            filter = tr("PDF Documents (*.pdf)");
            defaultSuffix = QStringLiteral("pdf");
            break;
    }

    QFileDialog dlg(this, tr("Export Figure"), QString(), filter);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setDefaultSuffix(defaultSuffix);

    if (!pathEdit_->text().isEmpty()) {
        dlg.selectFile(pathEdit_->text());
    }

    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        if (!files.isEmpty()) {
            pathEdit_->setText(files.first());
        }
    }
}

}  // namespace lumen::ui
