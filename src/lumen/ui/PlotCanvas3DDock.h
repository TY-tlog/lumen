#pragma once

#include <QDockWidget>

namespace lumen::ui {

class PlotCanvas3D;

/// Dock widget that hosts a PlotCanvas3D for 3D visualization.
///
/// Created by MainWindow and shown when 3D sample data is opened.
class PlotCanvas3DDock : public QDockWidget {
    Q_OBJECT

public:
    explicit PlotCanvas3DDock(QWidget* parent = nullptr);

    [[nodiscard]] PlotCanvas3D* canvas() const { return canvas_; }

private:
    PlotCanvas3D* canvas_ = nullptr;
};

}  // namespace lumen::ui
