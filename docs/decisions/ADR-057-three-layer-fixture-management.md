# ADR-057: Three-layer fixture management + renderer pinning

## Status
Proposed (Phase 9.5.1)

## Context
Vector consistency CI needs deterministic reference images to
compare against. Three concerns must be balanced:
1. **Repo size**: PNG reference images should not bloat the repo.
2. **Bisectability**: reference changes must be trackable.
3. **Reproducibility**: renderer versions must be pinned.

## Decision
Three-layer fixture architecture:

### Layer 1: Source fixtures (git-committed, text)
- Location: `tests/export/fixtures/source/*.lumen.json`
- 12 deterministic plot specifications (no external data deps)
- Small text files (< 50 KB each), normal git tracking
- Changes trigger CI; diffs are human-readable

### Layer 2: Renderer pinning
Renderers pinned to specific versions to prevent silent drift:

| Renderer | Pin | Rationale |
|----------|-----|-----------|
| Playwright Chromium | `playwright==1.49.0` (Chromium r1148) | Stable release as of 2026-04-14 |
| pdftocairo | `poppler-utils` from Ubuntu 22.04 apt (22.02.0) | Deterministic via runner OS pin |
| Inkscape | `inkscape` from Ubuntu 22.04 apt (1.1.2) | Deterministic via runner OS pin |

Version bumps are intentional (PR + re-generate references),
never automatic.

### Layer 3: Reference PNGs (Git LFS)
- Location: `tests/export/fixtures/reference/{viewer}/*.png`
- Tracked via Git LFS (`.gitattributes`)
- Update policy: regenerated ONLY at `vphase-*` tag commits
- CI workflow enforces: if reference files are modified in a
  non-tag commit, the workflow fails with an explicit message
- On comparison failure: CI uploads actual PNG + diff heatmap
  as workflow artifacts

### Fallback strategy
If a renderer is unavailable in CI (e.g., Docker pull failure):
- pdftocairo: fall back to `apt install poppler-utils`
- Inkscape: fall back to `apt install inkscape`
- Playwright: no fallback (required); CI fails with clear error

## Consequences
- + Repo stays small (PNGs in LFS, source fixtures in git)
- + Changes to references are auditable (LFS history + tag policy)
- + Renderer drift is impossible without explicit version bump
- + Diff artifacts on failure enable quick debugging
- - Git LFS setup required for contributors (one-time `git lfs install`)
- - Reference regeneration requires running all 3 renderers locally
  or in CI (script provided)
- - Ubuntu 22.04 pin limits CI runner flexibility

## Alternatives considered
- **No reference images** (compare renderers against each other):
  Rejected. All 3 renderers could drift together. Ground truth
  is needed.
- **Reference images in normal git**: Rejected. 36 PNGs × ~500KB
  each = ~18 MB per reference set. Accumulates rapidly with
  updates. LFS avoids this.
- **External reference storage** (S3, GCS): Rejected. Adds
  infrastructure dependency. Git LFS is simpler and sufficient
  within GitHub's free tier (1 GB).
- **Docker-only rendering** (pin exact image hashes): Considered
  for pdftocairo/Inkscape but rejected as overkill. Apt packages
  on a pinned Ubuntu version are sufficiently deterministic.
  Docker is the fallback, not the primary.
