#include "cascade.h"

namespace lumen::style {

StrokeStyle mergeStroke(const StrokeStyle& low, const StrokeStyle& high)
{
    return {
        mergeOpt(low.color, high.color),
        mergeOpt(low.width, high.width),
        mergeOpt(low.dash, high.dash),
        mergeOpt(low.cap, high.cap),
        mergeOpt(low.join, high.join),
    };
}

FillStyle mergeFill(const FillStyle& low, const FillStyle& high)
{
    return {
        mergeOpt(low.color, high.color),
        mergeOpt(low.alpha, high.alpha),
        mergeOpt(low.hatch, high.hatch),
    };
}

TextStyle mergeText(const TextStyle& low, const TextStyle& high)
{
    return {
        mergeOpt(low.family, high.family),
        mergeOpt(low.size, high.size),
        mergeOpt(low.weight, high.weight),
        mergeOpt(low.slant, high.slant),
        mergeOpt(low.color, high.color),
    };
}

MarkerStyle mergeMarker(const MarkerStyle& low, const MarkerStyle& high)
{
    return {
        mergeOpt(low.shape, high.shape),
        mergeOpt(low.size, high.size),
        mergeOpt(low.fillColor, high.fillColor),
        mergeOpt(low.strokeColor, high.strokeColor),
    };
}

GridStyle mergeGrid(const GridStyle& low, const GridStyle& high)
{
    return {
        mergeOpt(low.visible, high.visible),
        mergeOpt(low.majorColor, high.majorColor),
        mergeOpt(low.minorColor, high.minorColor),
        mergeOpt(low.majorWidth, high.majorWidth),
    };
}

namespace {

Optional<StrokeStyle> mergeOptStroke(const Optional<StrokeStyle>& low,
                                      const Optional<StrokeStyle>& high)
{
    if (!low.has_value() && !high.has_value()) return std::nullopt;
    if (!low.has_value()) return high;
    if (!high.has_value()) return low;
    return mergeStroke(*low, *high);
}

Optional<FillStyle> mergeOptFill(const Optional<FillStyle>& low,
                                  const Optional<FillStyle>& high)
{
    if (!low.has_value() && !high.has_value()) return std::nullopt;
    if (!low.has_value()) return high;
    if (!high.has_value()) return low;
    return mergeFill(*low, *high);
}

Optional<TextStyle> mergeOptText(const Optional<TextStyle>& low,
                                  const Optional<TextStyle>& high)
{
    if (!low.has_value() && !high.has_value()) return std::nullopt;
    if (!low.has_value()) return high;
    if (!high.has_value()) return low;
    return mergeText(*low, *high);
}

Optional<MarkerStyle> mergeOptMarker(const Optional<MarkerStyle>& low,
                                      const Optional<MarkerStyle>& high)
{
    if (!low.has_value() && !high.has_value()) return std::nullopt;
    if (!low.has_value()) return high;
    if (!high.has_value()) return low;
    return mergeMarker(*low, *high);
}

Optional<GridStyle> mergeOptGrid(const Optional<GridStyle>& low,
                                  const Optional<GridStyle>& high)
{
    if (!low.has_value() && !high.has_value()) return std::nullopt;
    if (!low.has_value()) return high;
    if (!high.has_value()) return low;
    return mergeGrid(*low, *high);
}

}  // namespace

Style mergeStyle(const Style& low, const Style& high)
{
    Style result;
    result.stroke = mergeOptStroke(low.stroke, high.stroke);
    result.fill = mergeOptFill(low.fill, high.fill);
    result.text = mergeOptText(low.text, high.text);
    result.marker = mergeOptMarker(low.marker, high.marker);
    result.grid = mergeOptGrid(low.grid, high.grid);
    result.backgroundColor = mergeOpt(low.backgroundColor, high.backgroundColor);
    result.foregroundColor = mergeOpt(low.foregroundColor, high.foregroundColor);
    result.lineWidth = mergeOpt(low.lineWidth, high.lineWidth);
    result.markerSize = mergeOpt(low.markerSize, high.markerSize);
    result.colormapName = mergeOpt(low.colormapName, high.colormapName);
    result.contourLevels = mergeOpt(low.contourLevels, high.contourLevels);
    result.barWidth = mergeOpt(low.barWidth, high.barWidth);
    return result;
}

Style cascade(const Style& theme, const Style& preset,
              const Style& plot, const Style& element)
{
    Style r = mergeStyle(theme, preset);
    r = mergeStyle(r, plot);
    r = mergeStyle(r, element);
    return r;
}

// --- Tracing helpers ---

namespace {

template <typename T>
void traceProperty(const QString& propName,
                   const Optional<T>& themeVal, const QString& themeName,
                   const Optional<T>& presetVal, const QString& presetName,
                   const Optional<T>& plotVal,
                   const Optional<T>& elementVal,
                   const QString& valueStr,
                   CascadeTrace& trace)
{
    CascadeLevel level;
    QString source;

    if (elementVal.has_value()) {
        level = CascadeLevel::ElementOverride;
        source = QStringLiteral("element override");
    } else if (plotVal.has_value()) {
        level = CascadeLevel::PlotInstance;
        source = QStringLiteral("plot instance");
    } else if (presetVal.has_value()) {
        level = CascadeLevel::Preset;
        source = QStringLiteral("preset (%1)").arg(presetName);
    } else if (themeVal.has_value()) {
        level = CascadeLevel::Theme;
        source = QStringLiteral("theme (%1)").arg(themeName);
    } else {
        return;  // No value at any level.
    }

    trace.append({propName, valueStr, level, source});
}

}  // namespace

Style cascadeWithTrace(const Style& theme, const QString& themeName,
                       const Style& preset, const QString& presetName,
                       const Style& plot,
                       const Style& element,
                       CascadeTrace& trace)
{
    Style resolved = cascade(theme, preset, plot, element);

    // Trace top-level scalar properties.
    if (resolved.backgroundColor.has_value()) {
        traceProperty(QStringLiteral("backgroundColor"),
                      theme.backgroundColor, themeName,
                      preset.backgroundColor, presetName,
                      plot.backgroundColor, element.backgroundColor,
                      resolved.backgroundColor->name(), trace);
    }
    if (resolved.foregroundColor.has_value()) {
        traceProperty(QStringLiteral("foregroundColor"),
                      theme.foregroundColor, themeName,
                      preset.foregroundColor, presetName,
                      plot.foregroundColor, element.foregroundColor,
                      resolved.foregroundColor->name(), trace);
    }
    if (resolved.lineWidth.has_value()) {
        traceProperty(QStringLiteral("lineWidth"),
                      theme.lineWidth, themeName,
                      preset.lineWidth, presetName,
                      plot.lineWidth, element.lineWidth,
                      QString::number(*resolved.lineWidth), trace);
    }

    // Trace stroke sub-properties.
    auto traceStroke = [&](const Optional<StrokeStyle>& ts,
                           const Optional<StrokeStyle>& ps,
                           const Optional<StrokeStyle>& pls,
                           const Optional<StrokeStyle>& es) {
        auto tv = ts.value_or(StrokeStyle{});
        auto pv = ps.value_or(StrokeStyle{});
        auto plv = pls.value_or(StrokeStyle{});
        auto ev = es.value_or(StrokeStyle{});
        if (resolved.stroke.has_value() && resolved.stroke->color.has_value()) {
            traceProperty(QStringLiteral("stroke.color"),
                          tv.color, themeName, pv.color, presetName,
                          plv.color, ev.color,
                          resolved.stroke->color->name(), trace);
        }
        if (resolved.stroke.has_value() && resolved.stroke->width.has_value()) {
            traceProperty(QStringLiteral("stroke.width"),
                          tv.width, themeName, pv.width, presetName,
                          plv.width, ev.width,
                          QString::number(*resolved.stroke->width), trace);
        }
    };
    traceStroke(theme.stroke, preset.stroke, plot.stroke, element.stroke);

    // Trace text sub-properties.
    if (resolved.text.has_value() && resolved.text->family.has_value()) {
        auto tv = theme.text.value_or(TextStyle{});
        auto pv = preset.text.value_or(TextStyle{});
        auto plv = plot.text.value_or(TextStyle{});
        auto ev = element.text.value_or(TextStyle{});
        traceProperty(QStringLiteral("text.family"),
                      tv.family, themeName, pv.family, presetName,
                      plv.family, ev.family,
                      *resolved.text->family, trace);
    }

    return resolved;
}

}  // namespace lumen::style
