# Phase 9.5 — Export Polish

**Status**: spec
**Predecessor**: Phase 9 (vphase-9, da56fc8)
**Successor**: Phase 10 (Style System)
**Scope**: Complete the items deferred from Phase 9. Establish an export regression safety net before Phase 10.

---

## 1. Motivation

At Phase 9 closing, two items were honestly marked as deferred:
- **Vector Consistency CI** (T9–T11, originally Phase 9.3): no cross-viewer SVG/PDF consistency verification.
- **Full MicroTeX**: only basic macros implemented; AMS math / matrix / align coverage insufficient.

These two gaps mean any export regression introduced in Phase 10+ could land silently on main. Phase 10 (Style System) does not touch the export pipeline directly, but Phase 11 (Dashboard) onward couples multi-plot composition tightly to export. Phase 9.5 installs that safety net first.

## 2. Sub-phase structure

| Sub | Title | Core deliverable |
|-----|-------|------------------|
| 9.5.1 | Vector Consistency CI Infrastructure | playwright + pdftocairo + comparison metric pipeline |
| 9.5.2 | SVG/PDF Strict Compliance | SVG 1.1 validator + PDF/A-2b basic compliance |
| 9.5.3 | Full MicroTeX | Tier 1/2 macro implementation + golden corpus verification |

Each sub-phase is an M-gate. Zero regression on prior phases.

---

## 3. Sub-phase 9.5.1 — Vector Consistency CI Infrastructure

### 3.1 Goal
Automatically verify on every PR that SVG/PDF exports render consistently across major viewers (headless Chromium, pdftocairo, Inkscape CLI).

### 3.2 Architecture (3-layer fixture)

**Layer 1 — Source fixtures (git-committed, text)**
- Location: `tests/export/fixtures/source/`
- Format: `.lumen.json` (Lumen internal plot spec) → CI renders to SVG/PDF
- Initial corpus: 12 fixtures
  - line_simple, line_log, scatter_color, bar_grouped, heatmap_viridis, heatmap_diverging, contour_filled, contour_lines, errorbar, stat_violin, plot3d_surface, plot3d_scatter
- Each fixture: deterministic seed, fixed viewport, fixed font (Inter, bundled)

**Layer 2 — Renderer pinning**
- Playwright Chromium: `playwright==1.49.0` lock, `chromium revision 1148` (CI env var)
- pdftocairo: Docker image `minidocks/poppler:25.04@sha256:<hash>`
- Inkscape: `inkscape:1.3.2` Docker image, hash pinned
- CI runner OS: **Ubuntu 22.04 only** for consistency CI. macOS/Windows runners do export smoke test only (file produced, non-corrupt).

**Layer 3 — Reference PNG (Git LFS)**
- Location: `tests/export/fixtures/reference/{viewer}/{fixture}.png` via Git LFS
- Update policy: regenerated and committed only at `vphase-*` tag commits. Reference changes in regular PRs require explicit reviewer approval.
- On failure, CI artifact: actual PNG + diff PNG (heatmap of |actual − reference|).

### 3.3 Comparison metrics (3-tier gate)

For every fixture × viewer pair:
gate_pass = (MS_SSIM >= 0.97) AND (PSNR_dB >= 35.0) AND
(CIEDE2000_mean <= 2.0) AND (CIEDE2000_max <= 5.0)

- **MS-SSIM**: scikit-image `structural_similarity` with multi-scale (5 levels). Suited for mixed text + color blocks.
- **PSNR**: 8-bit RGB, channel-averaged. Sanity check on global brightness drift.
- **CIEDE2000**: heatmap/contour fill regions only (extracted via alpha mask). Protects Phase 9 ICC work from regression. Uses `colour-science` Python package.

### 3.4 Text handling
Cross-viewer font matching is impossible (FreeType vs Pango vs Quartz hinting differences). Decision:
- **All SVG/PDF exports convert text to outline paths** (`QPainterPath` glyph outlines).
- Loss of selectable text is an explicit trade-off. Will be made optional in Phase 10 Style System (`export.text_as_path: bool` config).
- New ADR-055: "Text-as-path in publication export."

### 3.5 Deliverables
- `tests/export/fixtures/source/*.lumen.json` (12 files)
- `tests/export/render_runners/{playwright,pdftocairo,inkscape}.py`
- `tests/export/compare.py` (metric computation, JSON report)
- `tests/export/test_vector_consistency.py` (pytest, parameterized)
- `.github/workflows/vector-consistency.yml` (Linux only, Docker-based)
- ADR-055: text-as-path
- ADR-056: 3-tier metric gate rationale
- ADR-057: 3-layer fixture management

### 3.6 Exit checklist
- [ ] All 12 fixtures × 3 viewers = 36 comparisons pass the gate
- [ ] CI workflow auto-triggers on PR; on failure, diff artifact uploaded (verified)
- [ ] Reference PNG updates allowed only at tag commits (workflow-enforced)
- [ ] macOS/Windows smoke tests pass (file produced, non-corrupt)
- [ ] 762 → 800+ tests; all prior tests pass unchanged

---

## 4. Sub-phase 9.5.2 — SVG/PDF Strict Compliance

### 4.1 Goal
Verify export artifacts violate no standards. Prevents auto-rejection by journal submission systems (Elsevier, IEEE, etc.).

### 4.2 SVG 1.1 strict
- Validator: W3C `xmllint --noout --schema svg11.xsd`
- Every export SVG must pass
- Pre-audit common violations: duplicate `id`, deprecated `xlink:href` usage, missing `viewBox` on `<use>`

### 4.3 PDF/A-2b basic compliance
- Validator: `verapdf --flavour 2b`
- Pass condition: PDF/A-2b conformance level B (basic visual reproduction guarantee)
- Verify Phase 9 ICC profile embedding aligns with PDF/A requirement (output intent declaration). If misaligned, write ADR-058.

### 4.4 Deliverables
- `tests/export/test_svg_compliance.py`
- `tests/export/test_pdfa_compliance.py`
- verapdf Docker integration in CI workflow
- (conditional) ADR-058: PDF/A output intent

### 4.5 Exit checklist
- [ ] All 12 source fixtures pass SVG 1.1 validator
- [ ] All 12 source fixtures pass PDF/A-2b validator
- [ ] On CI failure, validator output uploaded as artifact

---

## 5. Sub-phase 9.5.3 — Full MicroTeX

### 5.1 Coverage tiers

**Tier 1 (100% required)** — covers ~95% of axis label / legend / title usage
_ ^  Greek (lowercase + uppercase)
\frac  \sqrt[n]{}  \sum  \int  \prod  \oint
\partial  \nabla  \infty
\pm  \mp  \times  \cdot  \div
\leq  \geq  \neq  \approx  \equiv  \sim  \propto
\mathrm  \mathbf  \mathit  \mathcal  \mathbb
\hat  \bar  \vec  \tilde  \dot  \ddot
\left( \right)  \left[ \right]  \left{ \right}  \left| \right|  \left. \right.

**Tier 2 (≥95% required)** — annotation equations
environments: align, aligned, cases, pmatrix, bmatrix, vmatrix, Vmatrix, matrix
\overline  \underline  \overbrace  \underbrace  \stackrel
\binom  \substack
\text{}  in math mode
\lim  \sup  \inf  \max  \min  \arg\max  \arg\min  \limsup  \liminf
\rightarrow  \leftarrow  \Rightarrow  \Leftarrow  \mapsto  \to

**Tier 3 (best-effort, known-gaps explicit)**
- `mhchem` chemistry (`\ce{}`) — unsupported, documented
- `\xrightarrow[below]{above}` — best-effort
- TikZ — unsupported (error message: "TikZ not supported in MicroTeX backend")

### 5.2 Golden corpus verification

**Corpus**: 60 equations in `tests/microtex/corpus/`
- 20: AMS short math sample (excerpts from `testmath.tex`)
- 40: Caption equations manually extracted from Nature / PRL / Neuron papers (last 5 years)

**Verification pipeline**:
1. Render via MicroTeX to PNG (300 DPI, white bg, black fg)
2. Render the same LaTeX via `pdflatex` subprocess (texlive Docker) → crop → PNG
3. SSIM comparison, threshold **0.85** (reflects MicroTeX vs LaTeX metric divergence)
4. Pass rate = coverage metric

**Exit metric**:
- Tier 1 corpus subset (20 equations): 100% pass
- Tier 2 corpus subset (30 equations): ≥95% (28/30) pass
- Tier 3 corpus subset (10 equations): best-effort, failures allowed, known gaps documented

### 5.3 LaTeX subprocess dependency
`texlive-full` Docker image (~5 GB) — used in CI only, not a Lumen runtime dependency. Local dev can skip corpus tests via env var `LUMEN_SKIP_LATEX_CORPUS=1`.

### 5.4 Deliverables
- Tier 1/2 macro implementation in `src/render/microtex/` (audit + fill gaps)
- `tests/microtex/corpus/*.tex` (60 files)
- `tests/microtex/test_corpus.py`
- `tests/microtex/test_tier1_unit.py` (Tier 1 macro unit tests)
- `.github/workflows/microtex-corpus.yml` (texlive Docker)
- ADR-059: MicroTeX coverage tier + golden corpus rationale
- ADR-060: SSIM 0.85 threshold (quantifies LaTeX vs MicroTeX metric divergence)
- `docs/microtex-coverage.md`: user-facing coverage doc, known gaps explicit

### 5.5 Exit checklist
- [ ] Tier 1 macro unit tests 100% pass
- [ ] Golden corpus Tier 1 subset 20/20 pass
- [ ] Golden corpus Tier 2 subset ≥28/30 pass
- [ ] `docs/microtex-coverage.md` published; Tier 3 known gaps documented
- [ ] No LaTeX dependency in Lumen runtime (only corpus tests use LaTeX)

---

## 6. Hard rules (verbatim, embedded in T-final)

### 6.1 Review-in-same-commit
At each sub-phase closing, `phase-9.5.{N}-review.md` and the closing `STATUS.md` entry land in the **same commit**. Stated verbatim in task body. This hard rule has held seven phases running (3b/4/5/6/7/8/9).

### 6.2 Per-deliverable verification in review
The review body contains, for **each user-visible item** in the sub-phase exit checklist, one line: "verified: X works (test/artifact path or command)." Hard rule introduced in Phase 9, enforced in Phase 9.5.

### 6.3 Branch check
Before every commit, run `git branch --show-current`. If output differs from intended branch, stop immediately. Lesson from Phase 8 stray-branch incident.

### 6.4 Zero prior regression
All 762 prior tests + every test added in each sub-phase pass unchanged. A single regression blocks the sub-phase.

---

## 7. Decisions locked

| ID | Decision | Rationale |
|----|----------|-----------|
| D1 | 3-tier metric gate (MS-SSIM 0.97 + PSNR 35 + CIEDE2000 2.0/5.0) | Single metric misses either font hinting or global drift |
| D2 | Text-as-path in vector export | Cross-viewer font match impossible; aligns with publication-grade promise |
| D3 | 3-layer fixture (source git + renderer pin + reference LFS, tag-only update) | Avoids repo bloat + preserves bisectability + ground truth is explicit |
| D4 | Vector CI Linux only | Isolates from macOS CoreText / Windows DWrite differences |
| D5 | MicroTeX 3-tier coverage, golden-corpus driven | "% of LaTeX" is unmeasurable; corpus pass rate is falsifiable |
| D6 | LaTeX comparison SSIM 0.85 | Realistic threshold quantifying MicroTeX vs Computer Modern metric divergence |

---

## 8. Out of scope (explicitly deferred to Phase 10+)

- `mhchem` chemistry rendering — Phase 13 (Computational Integration) or separate
- TikZ subset — permanent non-support, documented
- EPS export — largely deprecated in academia; reconsider on request
- Wide-gamut color (Display P3 / Rec. 2020) — Phase 9 sRGB/AdobeRGB judged sufficient
- macOS-native PDF rendering (CoreGraphics) — keep standard pipeline

---

## 9. Risk register

| Risk | Probability | Mitigation |
|------|-------------|------------|
| pdftocairo Docker image hash drift | M | Hash pin + Renovate bot for intentional updates only |
| LaTeX corpus too ambitious | M | Tier 3 failure allowed; explicit known-gaps |
| Text-as-path inflates SVG file size (3–10×) | H | Make optional in Phase 10. Current phase prioritizes publication-grade. |
| Reference PNG LFS cost | L | Updated only at tag commits; well within GitHub LFS free tier (1 GB) |
| MicroTeX upstream maintenance status | H | Maintain vendored fork. If needed, evaluate KaTeX-server backend (Phase 10+) |

---

## 10. Estimates

- Sub-phase 9.5.1: 5–7 days (CI infra is heaviest)
- Sub-phase 9.5.2: 2–3 days
- Sub-phase 9.5.3: 5–8 days (corpus collection + macro gap-fill)
- Total: ~2 weeks calendar, ~750 → ~830 tests projected

