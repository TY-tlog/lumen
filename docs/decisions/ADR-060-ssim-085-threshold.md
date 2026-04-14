# ADR-060: SSIM 0.85 threshold for MicroTeX vs LaTeX comparison

## Status
Accepted (Phase 9.5.3)

## Context
MathRenderer uses Unicode approximations for LaTeX rendering (not
a full TeX engine). Comparing its output to pdflatex reference
renderings requires a similarity threshold that accounts for:
- Font differences (system font vs Computer Modern)
- Layout differences (Unicode sub/superscripts vs TeX typesetting)
- Rendering engine differences (Qt QPainter vs TeX DVI)

## Decision
SSIM threshold of 0.85 for the golden corpus comparison.

### Rationale
- 0.85 is tight enough to catch "completely wrong" renderings
  (e.g., garbled text, missing symbols) which score < 0.6
- 0.85 is loose enough to accept the inherent Unicode-vs-TeX
  visual differences (font metrics, kerning, accent placement)
- Calibrated against Tier 1 corpus: Unicode renderings of
  standard math expressions score 0.80-0.92 vs pdflatex reference

### Comparison pipeline
1. Render via MathRenderer → PNG (300 DPI, white bg, black fg)
2. Render same LaTeX via pdflatex → crop → PNG
3. SSIM comparison (scikit-image structural_similarity)
4. Report pass rate per tier

### CI integration
- texlive-full Docker image for pdflatex reference (CI only)
- Local dev skips corpus tests via LUMEN_SKIP_LATEX_CORPUS=1
- No LaTeX runtime dependency in Lumen binary

## Consequences
- + Quantified quality metric for MathRenderer
- + Threshold is empirically calibrated
- + Tier failures are informative (know exactly which equations fail)
- - 0.85 may need adjustment as MathRenderer improves
- - texlive-full Docker image is ~5 GB (CI cost)

## Alternatives considered
- **PSNR threshold**: Rejected. PSNR is sensitive to global
  brightness; SSIM better captures structural similarity.
- **Exact pixel match**: Rejected. Impossible across different
  rendering engines.
- **Higher threshold (0.95)**: Rejected. Unicode approximation
  inherently differs from TeX typesetting. 0.95 would fail on
  most Tier 1 equations due to font metric differences.
