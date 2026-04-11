# ADR-034: Physical units with dimensional analysis

## Status
Accepted (Phase 6)

## Context
Scientific data has physical units (millivolts, milliseconds,
nanoamperes). Lumen v1 treats column names as free text with no
unit awareness. Phase 6 introduces a Unit class for dimensional
analysis: prevent unit errors, auto-generate axis labels, enable
unit conversion.

## Decision
Unit class with 7 SI base dimensions stored as integer exponents:

```
[length, mass, time, current, temperature, amount, luminosity]
```

Examples:
- meter: [1,0,0,0,0,0,0], scale=1.0
- millivolt: [2,1,-3,-1,0,0,0], scale=0.001 (relative to V)
- hertz: [0,0,-1,0,0,0,0], scale=1.0

Parsing grammar:
- "m" = meter (length dimension). NOT milli prefix.
- SI prefixes: k=1e3, M=1e6, G=1e9, m=1e-3 (only before a base
  symbol), μ=1e-6, n=1e-9, p=1e-12
- "mV" = milli + volt. "mm" = milli + meter.
- "m/s^2" = meter / second^2. Parsed left-to-right with `/` and
  `^` operators.
- Ambiguity rule: bare "m" = meter. "m" before a recognized
  base symbol = milli prefix.

Operator overloads: `*`, `/`, `pow(int)`. `isCompatible()` checks
dimension equality. `convert(value, target)` scales between
compatible units.

## Consequences
- + Axis labels auto-generated from units: "Voltage (mV)"
- + Unit mismatch detected at data load (e.g., plotting V vs mV
  triggers a warning or auto-conversion)
- + Dimensional analysis: V = I * R checks dimensionally
- + Pre-registered common units reduce parsing overhead
- - Parser is non-trivial (~200 lines); must handle edge cases
- - "m" ambiguity resolved by convention, not context. Users must
  write "mm" for millimeter, not "m*m" (which would be m²).

## Alternatives considered
- **String labels only**: column name "voltage_mV" is just text.
  No validation, no conversion. Rejected: misses the core value
  of units in scientific software.
- **Full computer algebra system**: symbolic unit manipulation with
  arbitrary expressions. Overkill for a plotting tool. Rejected.
- **Boost.Units**: mature C++ library but template-heavy, compile-
  time units only (no runtime parsing from file headers). Rejected.
