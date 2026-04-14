# Phase 9.5.1 Plan — Vector Consistency CI Infrastructure

**Spec**: `docs/specs/phase-9.5-spec.md` §3
**Architect**: Claude (Architect agent)
**Date**: 2026-04-14
**Baseline**: 762 tests, vphase-9 (da56fc8)

---

## Overview

Install an automated cross-viewer SVG/PDF consistency gate that
runs on every PR. 12 source fixtures rendered by 3 viewers
(headless Chromium, pdftocairo, Inkscape), compared via 3-tier
metric (MS-SSIM + PSNR + CIEDE2000) against git-committed
reference PNGs.

Text-as-path conversion ensures cross-viewer reproducibility by
eliminating font-hinting differences (ADR-055). This is the
structural prerequisite for Phase 10+ export changes.

## Hard constraints (verbatim from spec §6)

1. **Review-in-same-commit**: phase-9.5.1-review.md and closing
   STATUS entry in the SAME commit. Verbatim in T-final body.
2. **Per-deliverable verification**: review includes one line per
   exit checklist item confirming it works.
3. **Branch check**: `git branch --show-current` before every commit.
4. **Zero prior regression**: all 762 tests pass unchanged.

## Decisions resolved

### Playwright Chromium pin
- Playwright version: `1.49.0` (latest stable as of 2026-04-14)
- Chromium revision: `1148` (bundled with playwright 1.49.0)
- Locked via `pip install playwright==1.49.0` in CI
- Documented in ADR-057

### pdftocairo Docker image
- Image: `minidocks/poppler:25.04`
- Hash: resolved at first CI run; pinned in workflow via
  `docker pull minidocks/poppler:25.04@sha256:...`
- Fallback: if image unavailable, `apt install poppler-utils`
  on Ubuntu 22.04 (version 22.02.0)
- Documented in ADR-057

### Inkscape Docker
- Image: direct `apt install inkscape` on Ubuntu 22.04
  (version 1.1.2, deterministic on the runner)
- Docker alternative deferred unless apt version proves unstable

### Initial 12 fixture .lumen.json contents

Each fixture is a self-contained plot specification that Lumen's
`FigureExporter` can render without external data files. All use
deterministic generated data (seeded RNG or mathematical function).

| Fixture | Description | Data source |
|---------|-------------|-------------|
| `line_simple` | 3 sine waves, different phases | sin(x), sin(x+π/3), sin(x+2π/3), x∈[0,4π], 200 pts |
| `line_log` | Exponential decay, log-scale Y axis | exp(-x/τ) with τ=1,2,5, x∈[0,20], 100 pts |
| `scatter_color` | 200 points colored by Z value | x,y~N(0,1), z=x²+y² |
| `bar_grouped` | 4 categories × 3 groups | fixed values [3,7,5,2], [4,6,8,3], [5,4,3,6] |
| `heatmap_viridis` | 64×64 Gaussian | exp(-(x²+y²)/2σ²), Viridis colormap |
| `heatmap_diverging` | 64×64 saddle | x²−y², symmetric diverging colormap |
| `contour_filled` | 32×32 peaks function | Matlab peaks(32), filled contours, 10 levels |
| `contour_lines` | 32×32 Rosenbrock | (1-x)²+100(y-x²)², line contours, 8 levels |
| `errorbar` | 10 points with error bars | linear trend + Gaussian noise, ±1σ bars |
| `stat_violin` | 3 violin plots | 3 normal distributions, different μ and σ |
| `plot3d_surface` | Surface3D placeholder | 32×32 sinc, rendered as 2D heatmap fallback |
| `plot3d_scatter` | Scatter3D placeholder | 100 points on sphere, rendered as 2D XY scatter fallback |

Note: 3D fixtures render as 2D fallbacks for vector consistency
(3D OpenGL rendering is raster-only, not SVG/PDF). This is
acceptable because the vector CI validates the 2D export pipeline.

---

## Tasks

### T1: Text-as-path in FigureExporter
- **Owner**: Backend
- **Files**: src/lumen/plot/PlotRenderer.{h,cpp} (add
  textAsPath mode), src/lumen/core/io/FigureExporter.{h,cpp}
  (Options gains `textAsPath` bool, default true for SVG/PDF)
- **Acceptance**: When textAsPath=true, all `painter.drawText()`
  calls in PlotRenderer are replaced with
  `painter.drawPath(QPainterPath::addText(...))`. Exported SVG
  contains `<path>` elements instead of `<text>`. Exported PDF
  contains glyph outlines.
- **Size**: M
- **ADR**: ADR-055

### T2: 12 source fixture .lumen.json files
- **Owner**: Backend
- **Files**: tests/export/fixtures/source/*.lumen.json (12 files)
- **Acceptance**: Each fixture is a valid .lumen.json that can be
  loaded by WorkspaceFile and rendered by FigureExporter to
  produce a deterministic SVG and PDF. Fixtures use inline
  generated data (no external CSV dependency).
- **Size**: L
- **Note**: Fixtures must use the deterministic data generation
  approach — either encode data directly in JSON or use a
  fixture-generation script that produces identical output on
  every run (seeded RNG).

### T3: Playwright render runner
- **Owner**: Integration
- **Files**: tests/export/render_runners/playwright_runner.py
- **Acceptance**: Takes an SVG file path, renders via headless
  Chromium to PNG at 1050×700 @ 2x pixel ratio. Chromium
  version pinned via `playwright==1.49.0`.
- **Size**: S

### T4: pdftocairo render runner
- **Owner**: Integration
- **Files**: tests/export/render_runners/pdftocairo_runner.py
- **Acceptance**: Takes a PDF file path, renders to PNG via
  `pdftocairo -png -r 150`. Output resolution matches reference.
- **Size**: S

### T5: Inkscape render runner
- **Owner**: Integration
- **Files**: tests/export/render_runners/inkscape_runner.py
- **Acceptance**: Takes an SVG file path, renders via
  `inkscape --export-type=png --export-width=1050`. Version
  pinned in CI.
- **Size**: S

### T6: compare.py metric computation
- **Owner**: QA
- **Files**: tests/export/compare.py
- **Acceptance**: Takes two PNG paths, computes MS-SSIM
  (scikit-image), PSNR (numpy), CIEDE2000 mean+max
  (colour-science). Returns JSON: `{ms_ssim, psnr_db,
  ciede2000_mean, ciede2000_max, gate_pass}`. Gate pass when
  all 4 thresholds met (spec §3.3).
- **Size**: M
- **ADR**: ADR-056
- **Dependencies**: Python packages: scikit-image, numpy,
  colour-science, Pillow

### T7: test_vector_consistency.py
- **Owner**: QA
- **Files**: tests/export/test_vector_consistency.py
- **Acceptance**: pytest test, parameterized over 12 fixtures ×
  3 viewers = 36 test cases. Each: render fixture SVG/PDF →
  render via viewer → compare to reference PNG → assert gate.
  On failure: save actual PNG + diff heatmap as pytest artifact.
- **Size**: M

### T8: vector-consistency.yml CI workflow
- **Owner**: Integration
- **Files**: .github/workflows/vector-consistency.yml
- **Acceptance**: Triggers on PR to main. Ubuntu 22.04 runner.
  Installs: Playwright 1.49.0 (Chromium), poppler-utils
  (pdftocairo), Inkscape, Python deps. Runs
  test_vector_consistency.py. On failure: uploads diff artifacts.
  macOS/Windows: smoke test only (export produces file, no
  comparison).
- **Size**: L
- **ADR**: ADR-057

### T9: Reference PNG generation + Git LFS
- **Owner**: Integration
- **Files**: tests/export/fixtures/reference/{playwright,
  pdftocairo,inkscape}/*.png (36 files, Git LFS),
  tests/export/generate_references.py,
  .gitattributes (LFS tracking)
- **Acceptance**: generate_references.py renders all 12 fixtures
  × 3 viewers, producing 36 reference PNGs. PNGs committed via
  Git LFS. Script only runs at `vphase-*` tag commits (enforced
  by workflow check).
- **Size**: M

### T10: macOS/Windows smoke tests
- **Owner**: QA
- **Files**: .github/workflows/ci.yml (extended)
- **Acceptance**: On macOS and Windows CI jobs, export each of
  the 12 fixtures to SVG and PDF. Verify files are non-empty
  and non-corrupt (SVG starts with `<svg`, PDF starts with
  `%PDF-`). No metric comparison (viewer differences expected).
- **Size**: S

### T-final: ADRs + review + STATUS + close
- **Owner**: Architect + Docs
- **Files**: docs/decisions/ADR-055-text-as-path.md,
  docs/decisions/ADR-056-three-tier-metric-gate.md,
  docs/decisions/ADR-057-three-layer-fixture-management.md,
  docs/reviews/phase-9.5.1-review.md,
  .lumen-ops/STATUS.md
- **Acceptance**: phase-9.5.1-review.md WRITTEN AND COMMITTED IN
  THE SAME COMMIT AS THE CLOSING .lumen-ops/STATUS.md ENTRY.
  Review includes ONE-LINE VERIFICATION NOTE FOR EACH user-visible
  exit checklist item. `git branch --show-current` verified before
  commit. All 762 prior tests pass unchanged. vphase-9.5.1 tag.
- **Size**: M

---

## Critical path

```
T1 (text-as-path) ─→ T2 (fixtures) ─→ T9 (ref PNGs) ─→ T7 (tests)
                     T3 (playwright) ─┤
                     T4 (pdftocairo) ─┤
                     T5 (inkscape) ───┤
                     T6 (compare) ────┘
                                       T8 (CI workflow)
                                       T10 (smoke tests)
                                       T-final
```

T1 must land first (text-as-path changes SVG/PDF output format).
T2 depends on T1 (fixtures must render with text-as-path).
T3-T6 are independent of each other, can run in parallel.
T7 depends on T3-T6 and T9.
T8 depends on T7.
T-final closes.

## Test budget

| Source | Count |
|--------|-------|
| Phase 9 baseline | 762 |
| T1 text-as-path unit tests | 3 |
| T7 vector consistency (36 parametric) | 36 |
| T10 smoke tests | 2 |
| **Total** | **803** |

Target: 762 → 800+ (spec §3.6).

## Fixture data detail (T2 reference)

Each `.lumen.json` file follows workspace schema v1 with inline
data arrays. Example structure for `line_simple.lumen.json`:

```json
{
  "version": 1,
  "fixture": true,
  "plot": {
    "viewport": {"xmin": 0, "xmax": 12.57, "ymin": -1.2, "ymax": 1.2},
    "title": {"text": "Three Sine Waves", "fontPx": 17},
    "xAxis": {"label": "x (rad)", "rangeMode": "manual"},
    "yAxis": {"label": "Amplitude"},
    "legend": {"position": "top_right", "visible": true},
    "series": [
      {"type": "line", "name": "sin(x)", "color": "#0a84ff",
       "data": {"x": [0, 0.063, ...], "y": [0, 0.063, ...]}},
      ...
    ]
  }
}
```

Data arrays generated by `tests/export/generate_fixtures.py`
(deterministic, committed alongside fixtures).
