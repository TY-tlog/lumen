#pragma once

#include <QString>

#include <array>
#include <cstddef>
#include <stdexcept>

namespace lumen::data {

/// Physical unit with 7 SI base dimensions and scale factor.
///
/// Dimensions: [length, mass, time, current, temperature, amount, luminosity].
/// Supports parsing ("mV", "m/s^2"), arithmetic (*, /, pow), compatibility
/// checks, and unit conversion between compatible units.
class Unit {
public:
    /// Number of SI base dimensions.
    static constexpr std::size_t kDimCount = 7;

    /// Dimension indices for readability.
    enum DimIndex : std::size_t {
        Length = 0,
        Mass = 1,
        Time = 2,
        Current = 3,
        Temperature = 4,
        Amount = 5,
        Luminosity = 6,
    };

    /// Construct a unit from dimension exponents, scale, and symbol.
    Unit(std::array<int, kDimCount> dims, double scaleToSI, QString symbol);

    /// Construct a dimensionless unit (all exponents zero, scale 1.0).
    static Unit dimensionless();

    /// Parse a unit string.
    ///
    /// Grammar per ADR-034:
    /// - "m" = meter (bare m is always meter)
    /// - "mV" = milli + volt, "mm" = milli + meter
    /// - "m/s^2" = meter / second^2
    /// - SI prefixes: k=1e3, M=1e6, G=1e9, m=1e-3, u/\u00b5=1e-6, n=1e-9, p=1e-12
    /// - Pre-registered: V, mV, A, nA, pA, s, ms, m, mm, Hz, K, kg, mol, cd, Ohm, Pa, N, J, W
    ///
    /// Throws std::invalid_argument if the string cannot be parsed.
    static Unit parse(const QString& str);

    /// Dimension exponents.
    [[nodiscard]] const std::array<int, kDimCount>& dimensions() const { return dims_; }

    /// Conversion factor to base SI unit.
    [[nodiscard]] double scaleToSI() const { return scaleToSI_; }

    /// Display symbol.
    [[nodiscard]] const QString& symbol() const { return symbol_; }

    /// Two units are compatible if they have the same dimension exponents.
    [[nodiscard]] bool isCompatible(const Unit& other) const;

    /// Convert a value from this unit to the target unit.
    /// Throws std::invalid_argument if units are incompatible.
    [[nodiscard]] double convert(double value, const Unit& target) const;

    /// Multiply two units: exponents add, scales multiply.
    [[nodiscard]] Unit operator*(const Unit& rhs) const;

    /// Divide two units: exponents subtract, scales divide.
    [[nodiscard]] Unit operator/(const Unit& rhs) const;

    /// Raise unit to an integer power.
    [[nodiscard]] Unit pow(int exponent) const;

    /// Equality comparison (same dimensions, same scale).
    [[nodiscard]] bool operator==(const Unit& rhs) const;
    [[nodiscard]] bool operator!=(const Unit& rhs) const;

private:
    std::array<int, kDimCount> dims_;
    double scaleToSI_;
    QString symbol_;
};

} // namespace lumen::data
