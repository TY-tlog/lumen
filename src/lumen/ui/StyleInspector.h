#pragma once

#include "style/cascade.h"

#include <QDockWidget>

class QTableWidget;

namespace lumen::ui {

/// Side-panel dock showing resolved style properties with source levels.
/// Updated when user right-clicks an element → "Inspect style".
class StyleInspector : public QDockWidget {
    Q_OBJECT

public:
    explicit StyleInspector(QWidget* parent = nullptr);

    /// Update the display with a new cascade trace.
    void showTrace(const QString& elementName,
                   const style::CascadeTrace& trace);

    /// Clear the display.
    void clearTrace();

private:
    QTableWidget* table_ = nullptr;
};

}  // namespace lumen::ui
