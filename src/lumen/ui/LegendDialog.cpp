#include "LegendDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace lumen::ui {

LegendDialog::LegendDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Legend Properties"));

    auto* layout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout;

    // Position.
    positionCombo_ = new QComboBox(this);
    positionCombo_->addItem(tr("Top Left"), static_cast<int>(plot::LegendPosition::TopLeft));
    positionCombo_->addItem(tr("Top Right"), static_cast<int>(plot::LegendPosition::TopRight));
    positionCombo_->addItem(tr("Bottom Left"), static_cast<int>(plot::LegendPosition::BottomLeft));
    positionCombo_->addItem(tr("Bottom Right"), static_cast<int>(plot::LegendPosition::BottomRight));
    positionCombo_->addItem(tr("Outside Right"), static_cast<int>(plot::LegendPosition::OutsideRight));
    formLayout->addRow(tr("Position:"), positionCombo_);

    // Visible.
    visibleCheck_ = new QCheckBox(tr("Visible"), this);
    formLayout->addRow(QString(), visibleCheck_);

    layout->addLayout(formLayout);

    // Series table.
    seriesTable_ = new QTableWidget(this);
    seriesTable_->setColumnCount(2);
    seriesTable_->setHorizontalHeaderLabels({tr("Name"), tr("Color")});
    seriesTable_->horizontalHeader()->setStretchLastSection(true);
    seriesTable_->verticalHeader()->setVisible(false);
    seriesTable_->setSelectionMode(QAbstractItemView::NoSelection);
    layout->addWidget(seriesTable_);

    // OK / Cancel.
    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox_);
}

void LegendDialog::setLegendProperties(plot::LegendPosition position, bool visible,
                                        const QStringList& seriesNames,
                                        const QList<QColor>& seriesColors) {
    int posIdx = positionCombo_->findData(static_cast<int>(position));
    if (posIdx >= 0) {
        positionCombo_->setCurrentIndex(posIdx);
    }

    visibleCheck_->setChecked(visible);

    int rowCount = static_cast<int>(seriesNames.size());
    seriesTable_->setRowCount(rowCount);
    for (int i = 0; i < rowCount; ++i) {
        // Editable name cell.
        auto* nameItem = new QTableWidgetItem(seriesNames.at(i));
        nameItem->setFlags(nameItem->flags() | Qt::ItemIsEditable);
        seriesTable_->setItem(i, 0, nameItem);

        // Color swatch cell (read-only).
        auto* colorItem = new QTableWidgetItem;
        colorItem->setFlags(colorItem->flags() & ~Qt::ItemIsEditable);
        if (i < seriesColors.size()) {
            colorItem->setBackground(seriesColors.at(i));
        }
        seriesTable_->setItem(i, 1, colorItem);
    }

    seriesTable_->resizeColumnsToContents();
}

plot::LegendPosition LegendDialog::resultPosition() const {
    return static_cast<plot::LegendPosition>(positionCombo_->currentData().toInt());
}

bool LegendDialog::resultVisible() const {
    return visibleCheck_->isChecked();
}

QStringList LegendDialog::resultSeriesNames() const {
    QStringList names;
    for (int i = 0; i < seriesTable_->rowCount(); ++i) {
        auto* item = seriesTable_->item(i, 0);
        names.append(item != nullptr ? item->text() : QString());
    }
    return names;
}

}  // namespace lumen::ui
