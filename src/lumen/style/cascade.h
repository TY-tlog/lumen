#pragma once

#include "types.h"

#include <QString>
#include <QVector>

namespace lumen::style {

/// Source level in the cascade (for inspector tracing).
enum class CascadeLevel { Theme, Preset, PlotInstance, ElementOverride };

/// One resolved property with its source level.
struct CascadeEntry {
    QString property;
    QString value;
    CascadeLevel source;
    QString sourceName;  ///< e.g., theme name or "element override"
};

/// Full trace of a cascade resolution for the style inspector.
using CascadeTrace = QVector<CascadeEntry>;

/// Merge a single optional: higher priority wins.
template <typename T>
Optional<T> mergeOpt(const Optional<T>& low, const Optional<T>& high)
{
    return high.has_value() ? high : low;
}

/// Merge two StrokeStyles (per-property).
StrokeStyle mergeStroke(const StrokeStyle& low, const StrokeStyle& high);
FillStyle mergeFill(const FillStyle& low, const FillStyle& high);
TextStyle mergeText(const TextStyle& low, const TextStyle& high);
MarkerStyle mergeMarker(const MarkerStyle& low, const MarkerStyle& high);
GridStyle mergeGrid(const GridStyle& low, const GridStyle& high);

/// Merge two Styles (per-property, recursive into sub-styles).
Style mergeStyle(const Style& low, const Style& high);

/// Resolve the 4-level cascade: theme < preset < plot < element.
/// Returns the fully-resolved Style.
Style cascade(const Style& theme, const Style& preset,
              const Style& plot, const Style& element);

/// Resolve with tracing: returns resolved Style + CascadeTrace
/// showing which level each property came from.
Style cascadeWithTrace(const Style& theme, const QString& themeName,
                       const Style& preset, const QString& presetName,
                       const Style& plot,
                       const Style& element,
                       CascadeTrace& trace);

}  // namespace lumen::style
