#pragma once

#include <QColor>
#include <QString>

#include <optional>
#include <string>

namespace lumen::style {

template <typename T>
using Optional = std::optional<T>;

// --- Enums ---

enum class DashPattern { Solid, Dash, Dot, DashDot, DashDotDot };
enum class LineCap { Flat, Square, Round };
enum class LineJoin { Miter, Bevel, Round };
enum class HatchPattern { None, Horizontal, Vertical, Cross, DiagonalUp, DiagonalDown };
enum class FontWeight { Normal, Medium, DemiBold, Bold };
enum class FontSlant { Normal, Italic, Oblique };
enum class MarkerShape { Circle, Square, Triangle, Diamond, Cross, Plus };

// --- Sub-styles ---

struct StrokeStyle {
    Optional<QColor> color;
    Optional<double> width;
    Optional<DashPattern> dash;
    Optional<LineCap> cap;
    Optional<LineJoin> join;

    bool operator==(const StrokeStyle&) const = default;
};

struct FillStyle {
    Optional<QColor> color;
    Optional<double> alpha;
    Optional<HatchPattern> hatch;

    bool operator==(const FillStyle&) const = default;
};

struct TextStyle {
    Optional<QString> family;
    Optional<double> size;
    Optional<FontWeight> weight;
    Optional<FontSlant> slant;
    Optional<QColor> color;

    bool operator==(const TextStyle&) const = default;
};

struct MarkerStyle {
    Optional<MarkerShape> shape;
    Optional<double> size;
    Optional<QColor> fillColor;
    Optional<QColor> strokeColor;

    bool operator==(const MarkerStyle&) const = default;
};

struct GridStyle {
    Optional<bool> visible;
    Optional<QColor> majorColor;
    Optional<QColor> minorColor;
    Optional<double> majorWidth;

    bool operator==(const GridStyle&) const = default;
};

// --- Top-level Style ---

struct Style {
    Optional<StrokeStyle> stroke;
    Optional<FillStyle> fill;
    Optional<TextStyle> text;
    Optional<MarkerStyle> marker;
    Optional<GridStyle> grid;

    // Top-level properties
    Optional<QColor> backgroundColor;
    Optional<QColor> foregroundColor;
    Optional<double> lineWidth;
    Optional<double> markerSize;

    // Plot-type specific
    Optional<QString> colormapName;
    Optional<int> contourLevels;
    Optional<double> barWidth;

    bool operator==(const Style&) const = default;
};

}  // namespace lumen::style
