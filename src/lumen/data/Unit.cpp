#include "Unit.h"

#include <QHash>
#include <QString>

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

namespace lumen::data {

// ---- Construction ----

Unit::Unit(std::array<int, kDimCount> dims, double scaleToSI, QString symbol)
    : dims_(dims)
    , scaleToSI_(scaleToSI)
    , symbol_(std::move(symbol))
{
}

Unit Unit::dimensionless()
{
    return Unit({0, 0, 0, 0, 0, 0, 0}, 1.0, QStringLiteral(""));
}

// ---- Compatibility and conversion ----

bool Unit::isCompatible(const Unit& other) const
{
    return dims_ == other.dims_;
}

double Unit::convert(double value, const Unit& target) const
{
    if (!isCompatible(target)) {
        throw std::invalid_argument("Unit::convert: incompatible units '"
                                    + symbol_.toStdString() + "' and '"
                                    + target.symbol_.toStdString() + "'");
    }
    // value_in_SI = value * scaleToSI_
    // result = value_in_SI / target.scaleToSI_
    return value * scaleToSI_ / target.scaleToSI_;
}

// ---- Arithmetic ----

Unit Unit::operator*(const Unit& rhs) const
{
    std::array<int, kDimCount> newDims{};
    for (std::size_t i = 0; i < kDimCount; ++i) {
        newDims[i] = dims_[i] + rhs.dims_[i];
    }
    QString newSymbol = symbol_ + QStringLiteral("*") + rhs.symbol_;
    return Unit(newDims, scaleToSI_ * rhs.scaleToSI_, newSymbol);
}

Unit Unit::operator/(const Unit& rhs) const
{
    std::array<int, kDimCount> newDims{};
    for (std::size_t i = 0; i < kDimCount; ++i) {
        newDims[i] = dims_[i] - rhs.dims_[i];
    }
    QString newSymbol = symbol_ + QStringLiteral("/") + rhs.symbol_;
    return Unit(newDims, scaleToSI_ / rhs.scaleToSI_, newSymbol);
}

Unit Unit::pow(int exponent) const
{
    std::array<int, kDimCount> newDims{};
    for (std::size_t i = 0; i < kDimCount; ++i) {
        newDims[i] = dims_[i] * exponent;
    }
    double newScale = std::pow(scaleToSI_, exponent);
    QString newSymbol = symbol_ + QStringLiteral("^") + QString::number(exponent);
    return Unit(newDims, newScale, newSymbol);
}

// ---- Equality ----

bool Unit::operator==(const Unit& rhs) const
{
    return dims_ == rhs.dims_ && scaleToSI_ == rhs.scaleToSI_;
}

bool Unit::operator!=(const Unit& rhs) const
{
    return !(*this == rhs);
}

// ---- Parsing ----

namespace {

struct BaseUnit {
    QString symbol;
    std::array<int, Unit::kDimCount> dims;
    double scale;
};

// Pre-registered base units (no prefix applied).
// Volt: kg*m^2/(A*s^3)  → [2,1,-3,-1,0,0,0]
// Ampere: [0,0,0,1,0,0,0]
// Second: [0,0,1,0,0,0,0]
// Meter: [1,0,0,0,0,0,0]
// Hertz: [0,0,-1,0,0,0,0]
// Kelvin: [0,0,0,0,1,0,0]
// Kilogram: [0,1,0,0,0,0,0] (scale=1, "kg" is the SI base)
// Gram: [0,1,0,0,0,0,0] scale=0.001
// Mole: [0,0,0,0,0,1,0]
// Candela: [0,0,0,0,0,0,1]
// Ohm: kg*m^2/(A^2*s^3) → [2,1,-3,-2,0,0,0]
// Pascal: kg/(m*s^2) → [-1,1,-2,0,0,0,0]
// Newton: kg*m/s^2 → [1,1,-2,0,0,0,0]
// Joule: kg*m^2/s^2 → [2,1,-2,0,0,0,0]
// Watt: kg*m^2/s^3 → [2,1,-3,0,0,0,0]
const BaseUnit kBaseUnits[] = {
    {QStringLiteral("V"),   {2, 1, -3, -1, 0, 0, 0}, 1.0},
    {QStringLiteral("A"),   {0, 0, 0, 1, 0, 0, 0},   1.0},
    {QStringLiteral("s"),   {0, 0, 1, 0, 0, 0, 0},   1.0},
    {QStringLiteral("m"),   {1, 0, 0, 0, 0, 0, 0},   1.0},
    {QStringLiteral("Hz"),  {0, 0, -1, 0, 0, 0, 0},  1.0},
    {QStringLiteral("K"),   {0, 0, 0, 0, 1, 0, 0},   1.0},
    {QStringLiteral("kg"),  {0, 1, 0, 0, 0, 0, 0},   1.0},
    {QStringLiteral("g"),   {0, 1, 0, 0, 0, 0, 0},   0.001},
    {QStringLiteral("mol"), {0, 0, 0, 0, 0, 1, 0},   1.0},
    {QStringLiteral("cd"),  {0, 0, 0, 0, 0, 0, 1},   1.0},
    {QStringLiteral("Ohm"), {2, 1, -3, -2, 0, 0, 0}, 1.0},
    {QStringLiteral("Pa"),  {-1, 1, -2, 0, 0, 0, 0}, 1.0},
    {QStringLiteral("N"),   {1, 1, -2, 0, 0, 0, 0},  1.0},
    {QStringLiteral("J"),   {2, 1, -2, 0, 0, 0, 0},  1.0},
    {QStringLiteral("W"),   {2, 1, -3, 0, 0, 0, 0},  1.0},
};

struct PrefixEntry {
    QChar letter;
    double factor;
};

// SI prefixes (single-char). 'M' (mega) vs 'm' (milli) disambiguated by case.
const PrefixEntry kPrefixes[] = {
    {QChar('k'), 1e3},
    {QChar('M'), 1e6},
    {QChar('G'), 1e9},
    {QChar('m'), 1e-3},
    {QChar('u'), 1e-6},
    {QChar(0x00B5), 1e-6},  // µ
    {QChar('n'), 1e-9},
    {QChar('p'), 1e-12},
};

// Try to match a base unit symbol at the given position in the string.
// Returns the length of the match or 0 if no match.
qsizetype matchBaseUnit(const QString& str, qsizetype pos, const BaseUnit*& matched)
{
    qsizetype bestLen = 0;
    matched = nullptr;
    for (const auto& bu : kBaseUnits) {
        qsizetype len = bu.symbol.length();
        if (pos + len <= str.length() && str.mid(pos, len) == bu.symbol) {
            if (len > bestLen) {
                bestLen = len;
                matched = &bu;
            }
        }
    }
    return bestLen;
}

// Try to find a prefix at position pos, followed by a base unit.
// Returns total consumed length or 0.
qsizetype matchPrefixedUnit(const QString& str, qsizetype pos, double& outScale,
                             const BaseUnit*& outBase)
{
    if (pos >= str.length()) {
        return 0;
    }

    QChar ch = str[pos];

    for (const auto& pf : kPrefixes) {
        if (ch == pf.letter) {
            const BaseUnit* base = nullptr;
            qsizetype baseLen = matchBaseUnit(str, pos + 1, base);
            if (baseLen > 0) {
                outScale = pf.factor;
                outBase = base;
                return 1 + baseLen;
            }
        }
    }
    return 0;
}

// Parse a single unit token (possibly prefixed) at position pos.
// Returns the Unit and advances pos.
Unit parseSingleUnit(const QString& str, qsizetype& pos)
{
    if (pos >= str.length()) {
        throw std::invalid_argument("Unit::parse: unexpected end of string");
    }

    // Try bare base unit first (longest match)
    const BaseUnit* base = nullptr;
    qsizetype baseLen = matchBaseUnit(str, pos, base);

    // Try prefixed unit
    double prefixScale = 1.0;
    const BaseUnit* prefixedBase = nullptr;
    qsizetype prefixedLen = matchPrefixedUnit(str, pos, prefixScale, prefixedBase);

    // Ambiguity rule per ADR-034:
    // Bare "m" = meter. "mV" = milli + V. "mm" = milli + m.
    // If both bare and prefixed match, prefer prefixed (longer match),
    // UNLESS bare match is longer.
    if (prefixedLen > baseLen && prefixedBase != nullptr) {
        pos += prefixedLen;
        double scale = prefixedBase->scale * prefixScale;
        QString sym = str.mid(pos - prefixedLen, prefixedLen);
        return Unit(prefixedBase->dims, scale, sym);
    }
    if (baseLen > 0) {
        pos += baseLen;
        QString sym = base->symbol;
        return Unit(base->dims, base->scale, sym);
    }

    throw std::invalid_argument(
        "Unit::parse: unrecognized unit at position "
        + std::to_string(static_cast<long long>(pos))
        + " in '" + str.toStdString() + "'");
}

} // anonymous namespace

Unit Unit::parse(const QString& str)
{
    if (str.isEmpty()) {
        return dimensionless();
    }

    // Check for pre-registered compound units first.
    // This handles known multi-char units before the general parser.

    qsizetype pos = 0;
    Unit result = parseSingleUnit(str, pos);

    while (pos < str.length()) {
        QChar op = str[pos];

        if (op == QChar('*')) {
            ++pos;
            Unit rhs = parseSingleUnit(str, pos);
            result = result * rhs;
        } else if (op == QChar('/')) {
            ++pos;
            Unit rhs = parseSingleUnit(str, pos);
            // Check for ^exponent after the unit
            if (pos < str.length() && str[pos] == QChar('^')) {
                ++pos;
                // Parse integer exponent (possibly negative)
                qsizetype expStart = pos;
                if (pos < str.length() && str[pos] == QChar('-')) {
                    ++pos;
                }
                while (pos < str.length() && str[pos].isDigit()) {
                    ++pos;
                }
                if (pos == expStart) {
                    throw std::invalid_argument(
                        "Unit::parse: expected exponent after '^' in '"
                        + str.toStdString() + "'");
                }
                int exp = str.mid(expStart, pos - expStart).toInt();
                rhs = rhs.pow(exp);
            }
            result = result / rhs;
        } else if (op == QChar('^')) {
            ++pos;
            qsizetype expStart = pos;
            if (pos < str.length() && str[pos] == QChar('-')) {
                ++pos;
            }
            while (pos < str.length() && str[pos].isDigit()) {
                ++pos;
            }
            if (pos == expStart) {
                throw std::invalid_argument(
                    "Unit::parse: expected exponent after '^' in '"
                    + str.toStdString() + "'");
            }
            int exp = str.mid(expStart, pos - expStart).toInt();
            result = result.pow(exp);
        } else {
            throw std::invalid_argument(
                "Unit::parse: unexpected character '" + std::string(1, op.toLatin1())
                + "' at position " + std::to_string(static_cast<long long>(pos))
                + " in '" + str.toStdString() + "'");
        }
    }

    // Override symbol with original input for display
    result.symbol_ = str;
    return result;
}

} // namespace lumen::data
