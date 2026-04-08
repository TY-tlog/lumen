# ADR-015: Tick generation uses the 1-2-5 nice numbers algorithm

## Status
Accepted (Phase 2)

## Context
Axis tick marks must be placed at "human-friendly" values: 0, 100,
200, 300 rather than 0, 116.67, 233.33. The algorithm must handle
arbitrary data ranges including very small (0 to 1e-6), very large
(0 to 1e9), negative (-80 to +40), and fractional (-38.7 to -37.9)
ranges. It must also choose an appropriate number of decimal places
for tick labels.

## Decision
Implement the classic "nice numbers" algorithm (Heckbert 1990,
Graphics Gems):

1. Compute the rough tick spacing: roughSpacing = range / targetTickCount
2. Find the magnitude: mag = 10^floor(log10(roughSpacing))
3. Normalize: norm = roughSpacing / mag
4. Snap to nearest "nice" value in {1, 2, 5, 10}: this is the
   niceNum() function.
5. Compute niceSpacing = niceNum * mag
6. Round min down and max up to multiples of niceSpacing
7. Generate tick values from niceMin to niceMax in steps of
   niceSpacing
8. Compute decimal places from niceSpacing (e.g., spacing 0.1
   means 1 decimal place)

Default targetTickCount is 7 (produces 5-10 actual ticks depending
on range).

## Consequences
- + Produces universally readable tick labels for any range
- + Well-understood algorithm with 30+ years of use in plotting
- + Handles edge cases: zero range (add +/-0.5), very small
  ranges, very large ranges, negative ranges
- + Tick labels never have unnecessary trailing zeros
- + Deterministic: same input always produces same ticks
- - Tick range may extend slightly beyond data range (e.g., data
  [0, 700] produces ticks [0, 800]). This is standard behavior
  and matches user expectations from MATLAB, matplotlib, etc.
- - Does not support logarithmic tick spacing (deferred to Phase 3)
- - Fixed target count of 7; not adaptive to pixel width. Adequate
  for Phase 2 but should be made proportional to axis pixel
  length in Phase 4.

## Alternatives considered
- **Wilkinson's extended algorithm**: more sophisticated, produces
  slightly better results in edge cases, but significantly more
  complex. Overkill for Phase 2.
- **D3.js ticks algorithm**: similar to nice numbers but tuned for
  web. Would work, but we prefer the classic version for
  simplicity and C++ idiom.
- **Fixed tick count with linear spacing**: produces unreadable
  labels (e.g., 116.67). Rejected.
- **User-specified tick spacing**: useful as an override but
  automatic generation is the primary use case. User override
  deferred to Phase 5 (property inspector).
