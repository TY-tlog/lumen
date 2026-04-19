#pragma once

#include <QString>

namespace lumen::dashboard {

struct PanelConfig {
    int row = 0;
    int col = 0;
    int rowSpan = 1;
    int colSpan = 1;
    QString title;
    QString linkGroup = QStringLiteral("default");
};

}  // namespace lumen::dashboard
