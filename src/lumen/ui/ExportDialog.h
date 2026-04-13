#pragma once

#include <QDialog>
#include <QString>

class QButtonGroup;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;

namespace lumen::ui {

/// Dialog for configuring figure export options.
///
/// Presents format (PNG/SVG/PDF), size presets or custom dimensions,
/// DPI, background color, and output path.  Returns an ExportOptions
/// struct that can be converted to FigureExporter::Options.
class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget* parent = nullptr);

    struct ExportOptions {
        int format = 0;  // 0=PNG, 1=SVG, 2=PDF
        int widthPx = 1050;
        int heightPx = 700;
        int dpi = 300;
        bool transparentBackground = false;
        QString outputPath;
        int colorProfileIndex = 0;  ///< 0=sRGB, 1=AdobeRGB, 2=DisplayP3, 3=Custom
    };

    [[nodiscard]] ExportOptions options() const;

private:
    void updateControlVisibility();
    void updatePathExtension();
    void onBrowse();

    QButtonGroup* formatGroup_ = nullptr;
    QRadioButton* pngRadio_ = nullptr;
    QRadioButton* svgRadio_ = nullptr;
    QRadioButton* pdfRadio_ = nullptr;
    QComboBox* sizePreset_ = nullptr;
    QSpinBox* widthSpin_ = nullptr;
    QSpinBox* heightSpin_ = nullptr;
    QLabel* widthLabel_ = nullptr;
    QLabel* heightLabel_ = nullptr;
    QComboBox* dpiCombo_ = nullptr;
    QComboBox* bgCombo_ = nullptr;
    QLineEdit* pathEdit_ = nullptr;
    QPushButton* browseBtn_ = nullptr;
    QComboBox* profileCombo_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
