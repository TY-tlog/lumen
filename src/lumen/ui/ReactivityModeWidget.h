#pragma once

#include <core/reactive/ReactiveMode.h>

#include <QWidget>

class QButtonGroup;
class QRadioButton;

namespace lumen::ui {

/// Small toolbar widget with three radio buttons for selecting the
/// plot reactivity mode (Static / DAG / Bidirectional).
///
/// Emits modeChanged() when the user clicks a different button.
/// Call setMode() to update the selection programmatically.
class ReactivityModeWidget : public QWidget {
    Q_OBJECT

public:
    explicit ReactivityModeWidget(QWidget* parent = nullptr);

    /// Set the current selection programmatically (does not emit modeChanged).
    void setMode(reactive::Mode m);

    /// Return the currently selected mode.
    [[nodiscard]] reactive::Mode mode() const;

signals:
    void modeChanged(lumen::reactive::Mode m);

private:
    void onButtonToggled(int id, bool checked);

    QButtonGroup* group_ = nullptr;
    QRadioButton* staticBtn_ = nullptr;
    QRadioButton* dagBtn_ = nullptr;
    QRadioButton* bidiBtn_ = nullptr;
};

}  // namespace lumen::ui
