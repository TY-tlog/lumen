#pragma once

#include <QColor>
#include <QFont>

#include <array>
#include <cstdint>

namespace lumen::tokens {

// ---------------------------------------------------------------------------
// Color tokens — Light palette
// Source of truth: docs/design/design-system.md
// ---------------------------------------------------------------------------

namespace color {

namespace background {
inline constexpr QColor primary{255, 255, 255};    // #FFFFFF
inline constexpr QColor secondary{245, 245, 247};  // #F5F5F7
}  // namespace background

namespace surface {
inline constexpr QColor elevated{255, 255, 255};  // #FFFFFF
inline constexpr QColor sunken{236, 236, 238};    // #ECECEE
}  // namespace surface

namespace border {
inline constexpr QColor subtle{229, 229, 234};  // #E5E5EA
inline constexpr QColor strong{209, 209, 214};  // #D1D1D6
}  // namespace border

namespace text {
inline constexpr QColor primary{29, 29, 31};    // #1D1D1F
inline constexpr QColor secondary{99, 99, 102};  // #636366
inline constexpr QColor tertiary{142, 142, 147}; // #8E8E93
}  // namespace text

namespace accent {
inline constexpr QColor primary{10, 132, 255};  // #0A84FF
inline constexpr QColor muted{229, 240, 255};   // #E5F0FF
}  // namespace accent

inline constexpr QColor success{48, 209, 88};   // #30D158
inline constexpr QColor warning{255, 159, 10};  // #FF9F0A
inline constexpr QColor error{255, 59, 48};     // #FF3B30

// Plot categorical palette (color-blind-friendly)
inline constexpr std::array<QColor, 8> plotPalette{{
    {10, 132, 255},   // #0A84FF  blue
    {255, 159, 10},   // #FF9F0A  orange
    {48, 209, 88},    // #30D158  green
    {255, 55, 95},    // #FF375F  red
    {191, 90, 242},   // #BF5AF2  purple
    {94, 92, 230},    // #5E5CE6  indigo
    {100, 210, 255},  // #64D2FF  cyan
    {255, 214, 10},   // #FFD60A  yellow
}};

}  // namespace color

// ---------------------------------------------------------------------------
// Spacing — 4-unit baseline grid
// ---------------------------------------------------------------------------

namespace spacing {
inline constexpr int xxs = 4;
inline constexpr int xs = 8;
inline constexpr int sm = 12;
inline constexpr int md = 16;
inline constexpr int lg = 24;
inline constexpr int xl = 32;
inline constexpr int xxl = 48;
inline constexpr int xxxl = 64;

inline constexpr int containerPadding = 16;
inline constexpr int sectionGap = 24;
}  // namespace spacing

// ---------------------------------------------------------------------------
// Radii
// ---------------------------------------------------------------------------

namespace radius {
inline constexpr int xs = 4;   // chips, small badges
inline constexpr int sm = 8;   // buttons, inputs
inline constexpr int md = 12;  // cards, dialogs
inline constexpr int lg = 16;  // panels
}  // namespace radius

// ---------------------------------------------------------------------------
// Typography — sizes in px
// ---------------------------------------------------------------------------

namespace typography {

struct TypeStyle {
    int sizePx;
    QFont::Weight weight;
    int lineHeightPx;
};

inline constexpr TypeStyle caption{11, QFont::Normal, 14};
inline constexpr TypeStyle footnote{12, QFont::Normal, 16};
inline constexpr TypeStyle body{13, QFont::Normal, 18};
inline constexpr TypeStyle bodyStrong{13, QFont::Medium, 18};
inline constexpr TypeStyle subhead{15, QFont::Medium, 20};
inline constexpr TypeStyle title3{17, QFont::DemiBold, 22};
inline constexpr TypeStyle title2{22, QFont::DemiBold, 28};
inline constexpr TypeStyle title1{28, QFont::Bold, 34};

}  // namespace typography

// ---------------------------------------------------------------------------
// Shadows (as CSS-like strings for QSS)
// ---------------------------------------------------------------------------

namespace shadow {
// Note: QSS does not support box-shadow natively. These are stored
// for use in custom QPainter drawing.
// sm: 0 1px 2px rgba(0,0,0,0.04), 0 1px 1px rgba(0,0,0,0.06)
// md: 0 4px 12px rgba(0,0,0,0.08), 0 2px 4px rgba(0,0,0,0.06)
// lg: 0 12px 32px rgba(0,0,0,0.12), 0 4px 8px rgba(0,0,0,0.08)
// xl: 0 24px 64px rgba(0,0,0,0.16), 0 8px 16px rgba(0,0,0,0.10)
}  // namespace shadow

// ---------------------------------------------------------------------------
// Motion
// ---------------------------------------------------------------------------

namespace motion {
inline constexpr int durationFastMs = 120;
inline constexpr int durationNormalMs = 200;
inline constexpr int durationSlowMs = 320;
}  // namespace motion

// ---------------------------------------------------------------------------
// Plot defaults
// ---------------------------------------------------------------------------

namespace plot {
inline constexpr double defaultLineWidth = 1.5;
inline constexpr int markerSize = 5;
inline constexpr int axisLineWidth = 1;
inline constexpr int gridLineWidth = 1;
}  // namespace plot

// ---------------------------------------------------------------------------
// Application shell
// ---------------------------------------------------------------------------

namespace shell {
inline constexpr int toolbarHeight = 44;
}  // namespace shell

}  // namespace lumen::tokens
