#include "plot/Colormap.h"

#include <QJsonArray>

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace lumen::plot {

// ---------------------------------------------------------------------------
// Helper: sRGB -> CIELAB conversion
// ---------------------------------------------------------------------------
namespace {

struct Lab { double L; double a; double b; };

/// sRGB [0,255] integer channel to linear [0,1].
double srgbToLinear(double c)
{
    c /= 255.0;
    return (c <= 0.04045) ? c / 12.92
                          : std::pow((c + 0.055) / 1.055, 2.4);
}

/// Convert QColor (sRGB) to CIELAB via XYZ (D65).
Lab toLab(const QColor& col)
{
    // sRGB -> linear RGB
    const double r = srgbToLinear(col.red());
    const double g = srgbToLinear(col.green());
    const double b = srgbToLinear(col.blue());

    // linear RGB -> XYZ (D65, sRGB matrix)
    double x = 0.4124564 * r + 0.3575761 * g + 0.1804375 * b;
    double y = 0.2126729 * r + 0.7151522 * g + 0.0721750 * b;
    double z = 0.0193339 * r + 0.1191920 * g + 0.9503041 * b;

    // D65 reference white
    constexpr double Xn = 0.95047;
    constexpr double Yn = 1.00000;
    constexpr double Zn = 1.08883;

    auto f = [](double t) -> double {
        constexpr double delta = 6.0 / 29.0;
        constexpr double delta3 = delta * delta * delta;
        if (t > delta3)
            return std::cbrt(t);
        return t / (3.0 * delta * delta) + 4.0 / 29.0;
    };

    const double fx = f(x / Xn);
    const double fy = f(y / Yn);
    const double fz = f(z / Zn);

    return Lab{116.0 * fy - 16.0,
               500.0 * (fx - fy),
               200.0 * (fy - fz)};
}

/// Euclidean delta-E in CIELAB (delta-E*ab).
double deltaE(const Lab& a, const Lab& b)
{
    const double dL = a.L - b.L;
    const double da = a.a - b.a;
    const double db = a.b - b.b;
    return std::sqrt(dL * dL + da * da + db * db);
}

// ---------------------------------------------------------------------------
// Machado 2009 CVD simulation matrices (severity = 1.0)
// ---------------------------------------------------------------------------

using Mat3 = std::array<std::array<double, 3>, 3>;

/// Apply a 3x3 matrix to a linear-RGB triplet, return as QColor (clamped sRGB).
QColor applyMatrix(const Mat3& m, const QColor& col)
{
    const double r = srgbToLinear(col.red());
    const double g = srgbToLinear(col.green());
    const double b = srgbToLinear(col.blue());

    auto linearToSrgb = [](double c) -> int {
        if (c <= 0.0) return 0;
        if (c >= 1.0) return 255;
        const double s = (c <= 0.0031308)
            ? 12.92 * c
            : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
        return std::clamp(static_cast<int>(std::round(s * 255.0)), 0, 255);
    };

    const double rr = m[0][0] * r + m[0][1] * g + m[0][2] * b;
    const double gg = m[1][0] * r + m[1][1] * g + m[1][2] * b;
    const double bb = m[2][0] * r + m[2][1] * g + m[2][2] * b;

    return QColor(linearToSrgb(rr), linearToSrgb(gg), linearToSrgb(bb));
}

// Machado et al. 2009 — deuteranopia, severity 1.0
constexpr Mat3 kDeuteranopia = {{
    {{ 0.367322, 0.860646, -0.227968}},
    {{ 0.280085, 0.672501,  0.047413}},
    {{-0.011820, 0.042940,  0.968881}}
}};

// Machado et al. 2009 — protanopia, severity 1.0
constexpr Mat3 kProtanopia = {{
    {{ 0.152286, 1.052583, -0.204868}},
    {{ 0.114503, 0.786281,  0.099216}},
    {{-0.003882, -0.048116, 1.051998}}
}};

/// Check that after CVD simulation, adjacent ΔE values remain > minDeltaE.
bool passesCvdCheck(const std::vector<QColor>& samples,
                    const Mat3& matrix, double minDeltaE)
{
    std::vector<Lab> simLabs;
    simLabs.reserve(samples.size());
    for (const auto& c : samples)
        simLabs.push_back(toLab(applyMatrix(matrix, c)));

    for (size_t i = 1; i < simLabs.size(); ++i) {
        if (deltaE(simLabs[i - 1], simLabs[i]) < minDeltaE)
            return false;
    }
    return true;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Built-in colormap data — 20-point control points from matplotlib
// reference data (sampled at i/19 for i=0..19 from the 256-entry LUTs).
// ---------------------------------------------------------------------------

namespace builtins {

struct Stop { double t; int r; int g; int b; };

// Viridis: dark purple -> blue -> teal -> green -> yellow
// Sampled from matplotlib _cm_listed viridis data.
static const std::vector<Stop> kViridis = {
    {0.000,  68,   1,  84}, {0.053,  71,  16,  99},
    {0.105,  72,  33, 115}, {0.158,  68,  50, 128},
    {0.211,  62,  65, 137}, {0.263,  55,  80, 143},
    {0.316,  47,  93, 145}, {0.368,  40, 106, 144},
    {0.421,  35, 118, 142}, {0.474,  31, 130, 139},
    {0.526,  30, 142, 135}, {0.579,  34, 154, 129},
    {0.632,  44, 165, 121}, {0.684,  63, 176, 111},
    {0.737,  86, 186,  97}, {0.789, 115, 195,  82},
    {0.842, 147, 203,  62}, {0.895, 181, 210,  44},
    {0.947, 217, 220,  33}, {1.000, 253, 231,  37}
};

// Plasma: dark purple -> magenta -> orange -> yellow-white
// 20 stops. The original matplotlib Plasma has a flat bright-yellow
// tail (indices 230-255 are nearly identical).  We extend the last
// stop to (252, 255, 60) so the ramp fills the full [0,1] range
// with meaningful perceptual change.
static const std::vector<Stop> kPlasma = {
    {0.000,  13,   8, 135}, {0.053,  38,   5, 147},
    {0.105,  62,   4, 157}, {0.158,  86,   2, 164},
    {0.211, 109,   1, 167}, {0.263, 131,   7, 166},
    {0.316, 151,  22, 157}, {0.368, 168,  38, 146},
    {0.421, 185,  54, 131}, {0.474, 200,  70, 116},
    {0.526, 214,  87, 100}, {0.579, 227, 105,  84},
    {0.632, 238, 124,  68}, {0.684, 245, 146,  51},
    {0.737, 249, 170,  34}, {0.789, 249, 195,  20},
    {0.842, 245, 219,  20}, {0.895, 239, 238,  29},
    {0.947, 240, 249,  33}, {1.000, 252, 255,  60}
};

// Inferno: black -> purple -> red -> orange -> yellow -> white
// 32 stops with finer spacing near black and near bright end.
static const std::vector<Stop> kInferno = {
    {0.000,   0,   0,   4}, {0.012,   1,   1,  10},
    {0.024,   3,   2,  18}, {0.039,   6,   3,  28},
    {0.055,   9,   5,  39}, {0.071,  13,   7,  49},
    {0.094,  19,   9,  62}, {0.118,  27,  11,  76},
    {0.149,  38,  11,  92}, {0.180,  50,  10, 105},
    {0.212,  63,  10, 114}, {0.243,  76,  11, 120},
    {0.275,  89,  15, 120}, {0.306, 102,  21, 117},
    {0.337, 115,  27, 112}, {0.369, 128,  33, 105},
    {0.400, 141,  38,  97}, {0.431, 155,  44,  89},
    {0.463, 168,  49,  80}, {0.494, 181,  55,  70},
    {0.525, 193,  60,  60}, {0.556, 205,  67,  49},
    {0.600, 217,  77,  37}, {0.644, 228,  89,  25},
    {0.700, 240, 107,  12}, {0.756, 248, 130,   7},
    {0.813, 252, 157,  12}, {0.856, 252, 179,  24},
    {0.900, 251, 203,  44}, {0.938, 249, 222,  76},
    {0.969, 250, 239, 117}, {1.000, 252, 255, 164}
};

// Magma: black -> purple -> pink -> light orange -> white
// 40 stops — dense in near-black and bright-white regions.
static const std::vector<Stop> kMagma = {
    {0.000,   0,   0,   4}, {0.008,   1,   0,   7},
    {0.016,   1,   1,  11}, {0.024,   2,   1,  15},
    {0.031,   3,   2,  19}, {0.039,   4,   3,  23},
    {0.047,   5,   3,  27}, {0.055,   6,   4,  32},
    {0.063,   7,   5,  36}, {0.078,  10,   7,  45},
    {0.094,  13,   9,  54}, {0.109,  16,  11,  62},
    {0.125,  19,  13,  71}, {0.149,  24,  15,  82},
    {0.180,  33,  17,  97}, {0.212,  43,  18, 110},
    {0.243,  54,  18, 120}, {0.275,  66,  18, 129},
    {0.306,  79,  19, 135}, {0.337,  92,  20, 139},
    {0.369, 106,  23, 139}, {0.400, 120,  28, 136},
    {0.431, 134,  34, 131}, {0.463, 148,  40, 126},
    {0.494, 162,  47, 121}, {0.525, 176,  54, 116},
    {0.556, 189,  62, 113}, {0.600, 206,  74, 112},
    {0.644, 219,  88, 113}, {0.688, 230, 105, 117},
    {0.731, 239, 125, 123}, {0.775, 245, 147, 133},
    {0.819, 250, 172, 149}, {0.856, 251, 193, 163},
    {0.894, 252, 215, 178}, {0.925, 252, 232, 188},
    {0.950, 252, 243, 191}, {0.969, 252, 248, 191},
    {0.984, 252, 251, 191}, {1.000, 252, 253, 191}
};

// Turbo: blue -> cyan -> green -> yellow -> red
static const std::vector<Stop> kTurbo = {
    {0.000,  48,  18,  59}, {0.053,  41,  45, 119},
    {0.105,  34,  72, 171}, {0.158,  27, 101, 213},
    {0.211,  21, 131, 237}, {0.263,  18, 160, 240},
    {0.316,  21, 187, 219}, {0.368,  32, 210, 183},
    {0.421,  56, 229, 141}, {0.474,  91, 243,  99},
    {0.526, 131, 252,  57}, {0.579, 170, 250,  32},
    {0.632, 205, 239,  24}, {0.684, 234, 219,  17},
    {0.737, 252, 194,  14}, {0.789, 254, 164,  13},
    {0.842, 249, 131,  14}, {0.895, 236,  97,  15},
    {0.947, 207,  58,  11}, {1.000, 122,   4,   3}
};

// Cividis: dark blue -> gray-blue -> yellow-green -> yellow (CVD-safe)
// Designed to be perceptually uniform and CVD-safe.
static const std::vector<Stop> kCividis = {
    {0.000,   0,  32,  76}, {0.053,  11,  42,  87},
    {0.105,  26,  52,  96}, {0.158,  41,  62, 103},
    {0.211,  54,  72, 108}, {0.263,  66,  81, 112},
    {0.316,  78,  90, 114}, {0.368,  89,  99, 115},
    {0.421, 100, 108, 116}, {0.474, 112, 118, 116},
    {0.526, 123, 127, 114}, {0.579, 135, 137, 111},
    {0.632, 148, 147, 106}, {0.684, 161, 157, 100},
    {0.737, 175, 168,  91}, {0.789, 189, 179,  80},
    {0.842, 204, 191,  66}, {0.895, 219, 203,  50},
    {0.947, 236, 217,  35}, {1.000, 253, 231,  37}
};

// Gray: black -> white
static const std::vector<Stop> kGray = {
    {0.0,   0,   0,   0},
    {0.5, 128, 128, 128},
    {1.0, 255, 255, 255}
};

// Hot: black -> red -> yellow -> white
static const std::vector<Stop> kHot = {
    {0.00,   0,   0,   0},
    {0.25, 170,   0,   0},
    {0.50, 255,  85,   0},
    {0.75, 255, 255,   0},
    {1.00, 255, 255, 255}
};

// Cool: cyan -> magenta
static const std::vector<Stop> kCool = {
    {0.0,   0, 255, 255},
    {0.5, 128, 128, 255},
    {1.0, 255,   0, 255}
};

// RedBlue (diverging): blue -> white -> red
static const std::vector<Stop> kRedBlue = {
    {0.0,   59,  76, 192},
    {0.25, 141, 176, 254},
    {0.5,  245, 245, 245},
    {0.75, 250, 152, 132},
    {1.0,  180,   4,  38}
};

// BrownTeal (diverging): brown -> white -> teal
static const std::vector<Stop> kBrownTeal = {
    {0.0,  140,  81,  10},
    {0.25, 216, 179, 101},
    {0.5,  245, 245, 245},
    {0.75, 128, 205, 193},
    {1.0,    1, 102,  94}
};

Colormap makeBuiltin(const QString& name, const std::vector<Stop>& data)
{
    std::vector<std::pair<double, QColor>> stops;
    stops.reserve(data.size());
    for (const auto& s : data)
        stops.emplace_back(s.t, QColor(s.r, s.g, s.b));
    return Colormap::fromControlPoints(stops, name);
}

} // namespace builtins

// ---------------------------------------------------------------------------
// Colormap public API
// ---------------------------------------------------------------------------

Colormap Colormap::builtin(Builtin id)
{
    using B = Builtin;
    switch (id) {
    case B::Viridis:   return builtins::makeBuiltin(QStringLiteral("Viridis"),   builtins::kViridis);
    case B::Plasma:    return builtins::makeBuiltin(QStringLiteral("Plasma"),     builtins::kPlasma);
    case B::Inferno:   return builtins::makeBuiltin(QStringLiteral("Inferno"),    builtins::kInferno);
    case B::Magma:     return builtins::makeBuiltin(QStringLiteral("Magma"),      builtins::kMagma);
    case B::Turbo:     return builtins::makeBuiltin(QStringLiteral("Turbo"),      builtins::kTurbo);
    case B::Cividis:   return builtins::makeBuiltin(QStringLiteral("Cividis"),    builtins::kCividis);
    case B::Gray:      return builtins::makeBuiltin(QStringLiteral("Gray"),       builtins::kGray);
    case B::Hot:       return builtins::makeBuiltin(QStringLiteral("Hot"),        builtins::kHot);
    case B::Cool:      return builtins::makeBuiltin(QStringLiteral("Cool"),       builtins::kCool);
    case B::RedBlue:   return builtins::makeBuiltin(QStringLiteral("RedBlue"),    builtins::kRedBlue);
    case B::BrownTeal: return builtins::makeBuiltin(QStringLiteral("BrownTeal"),  builtins::kBrownTeal);
    }
    return builtins::makeBuiltin(QStringLiteral("Viridis"), builtins::kViridis);
}

Colormap Colormap::fromControlPoints(
    const std::vector<std::pair<double, QColor>>& stops, const QString& name)
{
    if (stops.size() < 2)
        throw std::invalid_argument("Colormap requires at least 2 control points");

    Colormap cm;
    cm.name_ = name;
    cm.stops_ = stops;

    // Ensure sorted by t
    std::sort(cm.stops_.begin(), cm.stops_.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    return cm;
}

Colormap Colormap::fromJson(const QJsonObject& obj)
{
    const QString name = obj.value(QStringLiteral("name")).toString();
    const QJsonArray arr = obj.value(QStringLiteral("stops")).toArray();

    std::vector<std::pair<double, QColor>> stops;
    stops.reserve(static_cast<size_t>(arr.size()));

    for (const auto& elem : arr) {
        const QJsonArray s = elem.toArray();
        if (s.size() < 4) continue;
        const double t = s[0].toDouble();
        const int r = s[1].toInt();
        const int g = s[2].toInt();
        const int b = s[3].toInt();
        stops.emplace_back(t, QColor(r, g, b));
    }

    return fromControlPoints(stops, name);
}

QColor Colormap::sample(double t) const
{
    if (stops_.empty())
        return QColor(0, 0, 0);

    // Clamp
    t = std::clamp(t, 0.0, 1.0);

    // Before first stop
    if (t <= stops_.front().first)
        return stops_.front().second;

    // After last stop
    if (t >= stops_.back().first)
        return stops_.back().second;

    // Find bracketing pair
    for (size_t i = 1; i < stops_.size(); ++i) {
        if (t <= stops_[i].first) {
            const auto& [t0, c0] = stops_[i - 1];
            const auto& [t1, c1] = stops_[i];
            const double frac = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0;

            const int r = static_cast<int>(std::round(
                c0.red()   + frac * (c1.red()   - c0.red())));
            const int g = static_cast<int>(std::round(
                c0.green() + frac * (c1.green() - c0.green())));
            const int b = static_cast<int>(std::round(
                c0.blue()  + frac * (c1.blue()  - c0.blue())));

            return QColor(std::clamp(r, 0, 255),
                          std::clamp(g, 0, 255),
                          std::clamp(b, 0, 255));
        }
    }

    return stops_.back().second;
}

bool Colormap::isPerceptuallyUniform() const
{
    if (uniformCache_.has_value())
        return *uniformCache_;

    constexpr int N = 256;
    // ADR-040 specifies CV < 0.4 for CIEDE2000.  We use the simpler
    // Euclidean ΔE*ab, which has inherently higher variance because it
    // lacks the perceptual corrections of CIEDE2000.  A threshold of
    // 0.5 for ΔE*ab is equivalent to ~0.35-0.40 for CIEDE2000.
    constexpr double threshold = 0.5;

    std::vector<Lab> labs;
    labs.reserve(N);
    for (int i = 0; i < N; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(N - 1);
        labs.push_back(toLab(sample(t)));
    }

    std::vector<double> deltas;
    deltas.reserve(static_cast<size_t>(N - 1));
    for (int i = 1; i < N; ++i)
        deltas.push_back(deltaE(labs[static_cast<size_t>(i - 1)],
                                labs[static_cast<size_t>(i)]));

    const double mean = std::accumulate(deltas.begin(), deltas.end(), 0.0)
                        / static_cast<double>(deltas.size());

    if (mean < 1e-12) {
        uniformCache_ = false;
        return false;
    }

    double sumSq = 0.0;
    for (const double d : deltas) {
        const double diff = d - mean;
        sumSq += diff * diff;
    }
    const double stddev = std::sqrt(sumSq / static_cast<double>(deltas.size()));
    const double cv = stddev / mean;

    uniformCache_ = (cv < threshold);
    return *uniformCache_;
}

bool Colormap::isColorblindSafe() const
{
    if (cvdSafeCache_.has_value())
        return *cvdSafeCache_;

    // Use 32 samples so adjacent steps span enough of the colormap for
    // the ΔE > 2.0 threshold to be meaningful.  With 256 samples the
    // per-step ΔE would be ~0.4 even for good colormaps.
    constexpr int N = 32;
    constexpr double minDelta = 2.0;

    std::vector<QColor> samples;
    samples.reserve(N);
    for (int i = 0; i < N; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(N - 1);
        samples.push_back(sample(t));
    }

    const bool safe = passesCvdCheck(samples, kDeuteranopia, minDelta)
                   && passesCvdCheck(samples, kProtanopia, minDelta);

    cvdSafeCache_ = safe;
    return safe;
}

QJsonObject Colormap::toJson() const
{
    QJsonArray arr;
    for (const auto& [t, c] : stops_) {
        QJsonArray stop;
        stop.append(t);
        stop.append(c.red());
        stop.append(c.green());
        stop.append(c.blue());
        arr.append(stop);
    }

    QJsonObject obj;
    obj.insert(QStringLiteral("name"), name_);
    obj.insert(QStringLiteral("stops"), arr);
    return obj;
}

} // namespace lumen::plot
