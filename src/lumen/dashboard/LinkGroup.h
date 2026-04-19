#pragma once

#include "LinkChannel.h"

#include <QObject>
#include <QString>
#include <QVector>

#include <array>

namespace lumen::dashboard {

class LinkGroup : public QObject {
    Q_OBJECT

public:
    explicit LinkGroup(const QString& name, QObject* parent = nullptr);

    [[nodiscard]] QString name() const { return name_; }

    void addPanel(int panelIndex);
    void removePanel(int panelIndex);
    [[nodiscard]] bool hasPanel(int panelIndex) const;
    [[nodiscard]] QVector<int> panels() const { return panels_; }

    void setChannelEnabled(LinkChannel ch, bool enabled);
    [[nodiscard]] bool isChannelEnabled(LinkChannel ch) const;

    void propagateXRange(int source, double xMin, double xMax);
    void propagateYRange(int source, double yMin, double yMax);
    void propagateCrosshair(int source, double dataX);
    void propagateSelection(int source, double selMin, double selMax);

signals:
    void xRangeChanged(int source, double xMin, double xMax);
    void yRangeChanged(int source, double yMin, double yMax);
    void crosshairMoved(int source, double dataX);
    void selectionChanged(int source, double selMin, double selMax);

private:
    static constexpr int kChannelCount = 4;

    QString name_;
    QVector<int> panels_;
    std::array<bool, kChannelCount> enabled_ = {true, false, true, false};
    bool propagating_ = false;
};

}  // namespace lumen::dashboard
