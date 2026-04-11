# ADR-040: Perceptual uniformity requirement for default colormaps

## Status
Accepted (Phase 7)

## Context
Rainbow colormaps (jet, hsv) create artificial features in scientific
data by mapping perceptually non-uniform color steps to uniform data
steps. Lumen's default colormaps must avoid this. Additionally,
~8% of males have color vision deficiency (CVD), so at least one
default map must be CVD-safe.

## Decision
Every built-in colormap is tested for perceptual uniformity at build
time (unit tests). The metric:

1. **Sample**: N=256 evenly spaced points along the colormap.
2. **Convert**: each sample to CIELAB color space (via sRGB → XYZ
   → CIELAB standard conversion).
3. **Compute**: ΔE₂₀₀₀ (CIEDE2000) between adjacent pairs.
4. **Check**: the coefficient of variation (CV = stddev / mean) of
   the ΔE series is below threshold T = 0.4.

CV < 0.4 means step sizes are reasonably uniform. Known-good maps
(viridis, plasma) score CV ≈ 0.1-0.2. Rainbow scores CV > 1.0.

`Colormap::isPerceptuallyUniform()` computes this at construction
and caches the result. The HeatmapPropertyDialog displays a
uniformity badge (checkmark or warning icon).

### Colorblind safety
CVD simulation uses Machado et al. 2009 color appearance model.
For each deficiency type (protanopia, deuteranopia, tritanopia):

1. Apply the 3×3 simulation matrix to each colormap sample.
2. Compute ΔE₂₀₀₀ between adjacent simulated colors.
3. Check: minimum ΔE > 2.0 (distinguishable under CVD).

`Colormap::isColorblindSafe()` returns true if all three deficiency
simulations pass. Cividis is designed to be CVD-safe and should
always pass.

## Consequences
- + Scientific validity: default colormaps don't create artifacts
- + Explicit metric: testable, reproducible, not subjective
- + CVD safety: cividis guaranteed safe, others tested
- + Badge in UI: user knows which maps are safe
- - CIELAB conversion adds ~1ms per colormap construction (acceptable)
- - Threshold T=0.4 is a heuristic; may need tuning if edge cases
  appear. Configurable in tests.
- - Custom user colormaps are NOT required to pass; they get a
  warning badge if they fail, not a rejection.

## Alternatives considered
- **No uniformity check** (any colormap allowed): rejected;
  undermines scientific validity which is Lumen's north-star.
- **Strict whitelist** (only pre-approved maps): rejected; users
  need custom colormaps for domain-specific visualization. The
  check is informational (badge), not blocking.
- **User acknowledges warning**: a middle ground (warn but allow).
  This IS the behavior for custom maps. Built-in maps must pass.
