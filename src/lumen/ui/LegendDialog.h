#pragma once

#include <plot/Legend.h>

#include <QColor>
#include <QDialog>
#include <QStringList>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QTableWidget;

namespace lumen::ui {

/// Dialog for editing legend properties.
///
/// Presents controls for position, visibility, and a table showing
/// each series name (editable) with its color swatch.
class LegendDialog : public QDialog {
    Q_OBJECT

public:
    explicit LegendDialog(QWidget* parent = nullptr);

    /// Populate the dialog controls from current legend properties.
    void setLegendProperties(plot::LegendPosition position, bool visible,
                             const QStringList& seriesNames,
                             const QList<QColor>& seriesColors);

    [[nodiscard]] plot::LegendPosition resultPosition() const;
    [[nodiscard]] bool resultVisible() const;
    [[nodiscard]] QStringList resultSeriesNames() const;

private:
    QComboBox* positionCombo_ = nullptr;
    QCheckBox* visibleCheck_ = nullptr;
    QTableWidget* seriesTable_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
