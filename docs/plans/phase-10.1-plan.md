# Phase 10.1 Plan — Cascade Engine + Schema

**Spec**: `docs/specs/phase-10-spec.md` §3
**Architect**: Claude (Architect agent)
**Date**: 2026-04-15
**Baseline**: 780 tests, vphase-9.5 (a4986b5)

---

## Overview

Implement the 4-level style cascade resolver, Style data structs,
JSON Schema v1.0, and the style inspector dev tool. This is the
foundation for all Phase 10 sub-phases.

## Hard constraints (verbatim from spec §7)

1. **Review-in-same-commit**: phase-10.1-review.md and closing
   STATUS entry in the SAME commit. Verbatim in T-final body.
2. **Per-deliverable verification**: review includes one line per
   exit checklist item confirming it works.
3. **Branch check**: `git branch --show-current` before every commit.
4. **Zero prior regression**: all 780 tests pass unchanged.
5. **Umbrella tag convention**: `vphase-10.1` at sub-phase close.
   `vphase-10` umbrella at final sub-phase (10.4) close.

## Open questions resolved

### Style struct v1.0 property set (~30 properties)

The spec lists StrokeStyle, FillStyle, TextStyle, MarkerStyle,
GridStyle. I propose the complete v1.0 property set:

**StrokeStyle** (5 props): color, width, dash, cap, join
**FillStyle** (3 props): color, alpha, hatch
**TextStyle** (5 props): family, size, weight, slant, color
**MarkerStyle** (4 props): shape, size, fillColor, strokeColor
**GridStyle** (4 props): visible, majorColor, minorColor, majorWidth

**Style** top-level (adds 9 props):
- stroke, fill, text, marker, grid (5 aggregate)
- backgroundColor (plot/figure background)
- foregroundColor (default text/line color)
- lineWidth (series default)
- markerSize (series default)

**Missing from spec — proposed additions**:
- `colormapName` (string, for heatmap/contour colormap binding)
- `contourLevels` (int, default auto-level count)
- `barWidth` (double, relative bar width)

**Total: ~33 properties.** Fits the spec's "~30" target.

### Token namespace

Reserved roots (dot-separated):
- `color.*` — all color tokens (primary, secondary, background, etc.)
- `font.*` — font family, size, weight
- `line.*` — line width, dash patterns
- `marker.*` — marker size, shape
- `grid.*` — grid visibility, colors
- `spacing.*` — padding, margins (future)

Naming convention: `{root}.{qualifier}.{detail}`
Examples: `color.primary`, `color.background`, `font.family`,
`font.size.default`, `font.size.title`, `line.width.default`,
`marker.size.default`

Documented in ADR-062.

### Style inspector UI

**Side-panel** (not modal). Rationale: modal blocks interaction;
the inspector needs to stay open while the user clicks different
elements to compare their resolved styles. Implemented as a
QDockWidget on the right side, toggled via View menu.

The spec §3.5 says "modal" but the usage pattern (right-click
element → inspect → click another element → compare) requires
persistent visibility. I propose side-panel with the spec's exact
column layout (Property / Value / Source).

---

## Tasks

### T1: Style data types
- **Owner**: Backend
- **Files**: src/lumen/style/types.h, src/lumen/style/types.cpp
- **Acceptance**: StrokeStyle, FillStyle, TextStyle, MarkerStyle,
  GridStyle, Style structs with Optional<T> for every property.
  Equality operators. Default construction leaves all nullopt.
- **Size**: M

### T2: Cascade resolver
- **Owner**: Backend
- **Files**: src/lumen/style/cascade.h, src/lumen/style/cascade.cpp
- **Acceptance**: `Style cascade(theme, preset, plot, element)`
  merges 4 levels with last-write-wins. Nullopt at any level
  falls through to next lower priority. Returns fully-resolved
  Style (no nullopts for properties that have a value at any level).
  Also: `CascadeTrace` variant that records `(property, value,
  source_level)` for the inspector.
- **Size**: M

### T3: JSON Schema v1.0
- **Owner**: Backend
- **Files**: schemas/style-v1.json
- **Acceptance**: JSON Schema draft-07 defining: $schema,
  lumen_style_version (required "1.0"), name, extends, tokens
  (object with string keys), elements (object with element
  namespace keys). Validates all placeholder themes.
- **Size**: S

### T4: JSON I/O (load/save with validation)
- **Owner**: Backend
- **Files**: src/lumen/style/json_io.h, src/lumen/style/json_io.cpp
- **Acceptance**: `Style loadStyleFromJson(QJsonObject)`,
  `QJsonObject saveStyleToJson(Style)`. Round-trip: Style → JSON
  → Style produces identical result. Token references (`$tokens.X`)
  resolved during load. Schema validation: reject files with
  missing required fields or unknown version.
- **Size**: M

### T5: Style inspector (side-panel)
- **Owner**: Frontend
- **Files**: src/lumen/ui/StyleInspector.h,
  src/lumen/ui/StyleInspector.cpp
- **Acceptance**: QDockWidget showing a QTableView with columns:
  Property, Value, Source. Populated by CascadeTrace when user
  right-clicks an element → "Inspect style". Source column shows
  "theme", "preset", "plot instance", or "element override" with
  the theme/preset name. Toggle via View menu.
- **Size**: L

### T6: Cascade unit tests
- **Owner**: QA
- **Files**: tests/unit/test_cascade.cpp
- **Acceptance**: Tests for:
  - 4-level priority (element > plot > preset > theme)
  - Nullopt fall-through (element has nullopt → plot value used)
  - Full override (every property set at element → all from element)
  - Empty cascade (all nullopt → default Style)
  - CascadeTrace records correct source levels
  - Mixed: some props from theme, some from plot, some from element
- **Size**: M (target: 12+ tests)

### T7: Schema validation tests
- **Owner**: QA
- **Files**: tests/unit/test_schema_validation.cpp
- **Acceptance**: Valid theme file passes. Missing version fails.
  Unknown version ("2.0") fails with clear error. Missing required
  fields fail. Extra unknown fields are tolerated (forward compat).
- **Size**: S (target: 6+ tests)

### T8: JSON roundtrip tests
- **Owner**: QA
- **Files**: tests/unit/test_json_roundtrip.cpp
- **Acceptance**: Style → JSON → Style produces identical Style.
  Token references resolve correctly. Nullopt properties omitted
  from JSON. Loaded Style has correct values.
- **Size**: S (target: 5+ tests)

### T-final: ADRs + review + STATUS + close
- **Owner**: Architect + Docs
- **Files**: docs/decisions/ADR-061-cascade-architecture.md,
  docs/decisions/ADR-062-json-schema-v1.md,
  docs/reviews/phase-10.1-review.md,
  .lumen-ops/STATUS.md
- **Acceptance**: phase-10.1-review.md WRITTEN AND COMMITTED IN
  THE SAME COMMIT AS THE CLOSING .lumen-ops/STATUS.md ENTRY.
  Review includes ONE-LINE VERIFICATION NOTE FOR EACH user-visible
  exit checklist item. `git branch --show-current` verified before
  commit. All 780 prior tests pass unchanged. `vphase-10.1` tag.
- **Size**: M

---

## Critical path

```
T1 (types) ─→ T2 (cascade) ─→ T5 (inspector)
          ├─→ T3 (schema)
          └─→ T4 (json_io) ─→ T8 (roundtrip tests)
               T6 (cascade tests) depends on T2
               T7 (schema tests) depends on T3+T4
               T-final after all
```

T1 first (types needed by everything).
T2 depends on T1. T3 and T4 depend on T1.
T5 depends on T2 (needs CascadeTrace).
T6-T8 parallel after their dependencies.

## Test budget

| Source | Count |
|--------|-------|
| Phase 9.5 baseline | 780 |
| T6 cascade tests | ~12 |
| T7 schema tests | ~6 |
| T8 roundtrip tests | ~5 |
| T5 inspector tests | ~3 |
| **Total** | **~806** |

Target: 780 → ~810 (spec §3.7).

## Style inspector detail (T5)

Layout:
```
┌─ Style Inspector ─────────────────────────────┐
│ Element: series.line (plot "fig1", trace 0)    │
│                                                │
│ Property          Value       Source            │
│ ─────────────────────────────────────────────  │
│ stroke.color      #0a84ff     element override  │
│ stroke.width      1.5pt       theme (lumen-light)│
│ fill.color        (none)      —                 │
│ text.family       Inter       theme (lumen-light)│
│ marker.shape      circle      preset (scatter)   │
│ ...                                             │
└────────────────────────────────────────────────┘
```

Right-click element → "Inspect style" updates the dock content.
Dock stays visible until toggled off via View menu.
