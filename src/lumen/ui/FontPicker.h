#pragma once

#include <QWidget>

class QComboBox;
class QLabel;

namespace lumen::ui {

/// Font picker with built-in academic fonts and system font option.
class FontPicker : public QWidget {
    Q_OBJECT

public:
    explicit FontPicker(QWidget* parent = nullptr);

    [[nodiscard]] QString selectedFamily() const;

signals:
    void fontChanged(const QString& family);

private:
    void onComboChanged(int index);

    QComboBox* combo_ = nullptr;
    QLabel* preview_ = nullptr;
};

}  // namespace lumen::ui
