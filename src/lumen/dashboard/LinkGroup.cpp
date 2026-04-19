#include "LinkGroup.h"

namespace lumen::dashboard {

LinkGroup::LinkGroup(const QString& name, QObject* parent)
    : QObject(parent)
    , name_(name)
{
}

void LinkGroup::addPanel(int panelIndex)
{
    if (!panels_.contains(panelIndex))
        panels_.append(panelIndex);
}

void LinkGroup::removePanel(int panelIndex)
{
    panels_.removeAll(panelIndex);
}

bool LinkGroup::hasPanel(int panelIndex) const
{
    return panels_.contains(panelIndex);
}

void LinkGroup::setChannelEnabled(LinkChannel ch, bool enabled)
{
    enabled_[static_cast<std::size_t>(ch)] = enabled;
}

bool LinkGroup::isChannelEnabled(LinkChannel ch) const
{
    return enabled_[static_cast<std::size_t>(ch)];
}

void LinkGroup::propagateXRange(int source, double xMin, double xMax)
{
    if (propagating_) return;
    if (!enabled_[static_cast<std::size_t>(LinkChannel::XAxis)]) return;
    if (!hasPanel(source)) return;

    propagating_ = true;
    emit xRangeChanged(source, xMin, xMax);
    propagating_ = false;
}

void LinkGroup::propagateYRange(int source, double yMin, double yMax)
{
    if (propagating_) return;
    if (!enabled_[static_cast<std::size_t>(LinkChannel::YAxis)]) return;
    if (!hasPanel(source)) return;

    propagating_ = true;
    emit yRangeChanged(source, yMin, yMax);
    propagating_ = false;
}

void LinkGroup::propagateCrosshair(int source, double dataX)
{
    if (propagating_) return;
    if (!enabled_[static_cast<std::size_t>(LinkChannel::Crosshair)]) return;
    if (!hasPanel(source)) return;

    propagating_ = true;
    emit crosshairMoved(source, dataX);
    propagating_ = false;
}

void LinkGroup::propagateSelection(int source, double selMin, double selMax)
{
    if (propagating_) return;
    if (!enabled_[static_cast<std::size_t>(LinkChannel::Selection)]) return;
    if (!hasPanel(source)) return;

    propagating_ = true;
    emit selectionChanged(source, selMin, selMax);
    propagating_ = false;
}

}  // namespace lumen::dashboard
