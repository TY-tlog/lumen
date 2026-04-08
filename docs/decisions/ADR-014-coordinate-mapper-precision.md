# ADR-014: CoordinateMapper uses double precision with linear interpolation

## Status
Accepted (Phase 2)

## Context
The plot engine maps between data coordinates (scientific values
like voltage in mV or time in ms) and pixel coordinates (integer
screen positions). The reference data has values like -38.226183 mV
and 0.2 ms time steps. The mapping must be precise enough that:

1. Round-tripping (data to pixel to data) preserves the original
   value to scientific precision.
2. Zooming in deeply (e.g., to a 0.1 mV range) still renders
   correctly without jitter.
3. Panning at high zoom does not accumulate drift.

## Decision
CoordinateMapper uses double for all computations and performs
simple linear interpolation:

```
pixelX = plotArea.x() + (dataX - xMin) / (xMax - xMin) * plotArea.width()
pixelY = plotArea.bottom() - (dataY - yMin) / (yMax - yMin) * plotArea.height()
```

Y is inverted (data Y increases upward, pixel Y increases downward).

No affine matrix, no integer rounding until final QPainter calls.
Round-trip precision target: within 1e-10 of original values (unit
tested).

## Consequences
- + Simple, fast, correct for scientific ranges
- + Double precision gives ~15 significant digits, far more than
  any scientific dataset needs
- + Y inversion is a single subtraction, not a matrix multiply
- + Round-trip accuracy verified by unit tests
- - Division by (xMax - xMin) produces infinity if range is zero;
  guarded by ViewTransform which ensures non-zero ranges
- - No support for logarithmic axes (requires different mapping);
  deferred to Phase 3 per spec

## Alternatives considered
- **QTransform matrix**: Qt's 2D transform matrix could handle the
  mapping, but adds complexity (matrix inversion for pixelToData)
  and obscures the simple linear relationship. No benefit for
  linear axes.
- **Integer pixel coordinates**: rounding to int early causes
  visible jitter at high zoom. Keeping double until QPainter
  (which accepts QPointF) avoids this entirely.
- **Separate X and Y mapper objects**: over-engineering for a
  linear axis. One mapper with both dimensions is simpler.
