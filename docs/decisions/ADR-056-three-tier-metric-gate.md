# ADR-056: Three-tier metric gate for vector consistency

## Status
Proposed (Phase 9.5.1)

## Context
A single image comparison metric is insufficient for scientific
plot verification:
- SSIM alone misses global brightness/color shifts (high
  structural similarity but wrong colors).
- PSNR alone misses structural differences (similar pixel values
  but misaligned features).
- Neither captures perceptual color accuracy for colormapped
  regions (heatmaps, contour fills).

## Decision
Three-tier gate: all four conditions must pass simultaneously.

```
gate_pass = (MS_SSIM >= 0.97)
        AND (PSNR_dB >= 35.0)
        AND (CIEDE2000_mean <= 2.0)
        AND (CIEDE2000_max <= 5.0)
```

### Tier 1: MS-SSIM ≥ 0.97
Multi-scale structural similarity (5 levels). Captures text
rendering quality, line crispness, marker shape fidelity. The
0.97 threshold was calibrated against text-as-path renderings
of the 12 canonical fixtures.

### Tier 2: PSNR ≥ 35.0 dB
8-bit RGB, channel-averaged. Sanity check on global brightness
and color drift. 35 dB corresponds to imperceptible differences
at normal viewing distance.

### Tier 3: CIEDE2000 mean ≤ 2.0, max ≤ 5.0
Applied to colormapped regions only (heatmap/contour fill areas
extracted via alpha mask). Protects Phase 9 ICC color management
work from regression. Uses the colour-science Python package for
CIEDE2000 computation.

Mean ≤ 2.0: average color difference is below "just noticeable"
threshold. Max ≤ 5.0: no single pixel exceeds "clearly
different" threshold.

### Implementation
`tests/export/compare.py` computes all four metrics and returns
a JSON report. CI workflow reads the report and fails the job if
any threshold is violated.

## Consequences
- + Catches structural, brightness, and color regressions
- + Each metric has clear physical interpretation
- + Thresholds are documented and adjustable per-fixture if needed
- - Three Python dependencies (scikit-image, numpy, colour-science)
- - CIEDE2000 computation is slower (~2s per 1050×700 image)
- - Threshold tuning may be needed as the rendering pipeline evolves

## Alternatives considered
- **Single PSNR threshold**: Rejected. Misses structural issues
  (e.g., a line shifted by 1px has high PSNR but low SSIM).
- **Single SSIM threshold**: Rejected. Misses color accuracy
  (SSIM is structural, not colorimetric).
- **Perceptual hash (pHash)**: Rejected. Binary pass/fail with
  no graduated metric. Can't distinguish "close miss" from
  "complete failure."
- **Human visual inspection**: Rejected. Not scalable, not
  reproducible.
