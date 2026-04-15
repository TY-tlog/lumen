# Phase 10 — Style System

**Status**: spec
**Predecessor**: Phase 9.5 (vphase-9.5, a4986b5)
**Successor**: Phase 11 (Dashboard + Linked Views)
**Scope**: Unified style cascade (theme → preset → plot → element), 6 bundled themes, live editing with explicit promotion, versioned JSON style format.

---

## 1. Motivation

Phase 9.5 closed the publication-grade export safety net. Phase 10 fills the next missing capability: **systematic visual control across plots and projects**. Currently every plot styles itself in isolation. There is no way to:
- Apply a consistent visual language across a project (every plot in a paper)
- Switch between presentation/publication/colorblind contexts without per-plot rework
- Save a style and reapply it elsewhere
- Inherit and override at the right granularity

Phase 10 is the first phase that touches the **plot specification** itself rather than wrapping it (Phase 6–9 added rendering, export, and 3D *around* the spec). This is load-bearing for Phase 11 (Dashboard) which needs theme inheritance across linked plots.

## 2. Sub-phase structure

| Sub | Title | Core deliverable |
|-----|-------|------------------|
| 10.1 | Cascade Engine + Schema | Style struct, 4-level cascade resolver, JSON Schema v1.0, dev tool |
| 10.2 | Theme Catalog | 6 bundled themes, vector CI fixture extension (12 → 72 references) |
| 10.3 | Live Editing + Promotion | Double-click integration, 3-toggle promotion, undo, immutability guards |
| 10.4 | Style Import/Export UI | File load/save, `extends` resolution, copy/paste style actions |

Each sub-phase is an M-gate. Zero regression on prior phases.

---

## 3. Sub-phase 10.1 — Cascade Engine + Schema

### 3.1 Goal
Implement the core 4-level style cascade and JSON serialization schema. All other sub-phases depend on this.

### 3.2 Cascade architecture (locked: D10.2)

Resolution order, lowest-to-highest priority:
resolved_style[element] = merge(
theme_default,           // priority 1 (lowest)
preset_default,          // priority 2
plot_instance_style,     // priority 3
element_override         // priority 4 (highest)
)

Per-property `last-write-wins`. **No CSS specificity computation.** A property defined at any level is overridden by the same property defined at any higher level. Properties unset at a level (`std::nullopt`) fall through.

### 3.3 Style data model

```cpp
namespace lumen::style {

template <typename T>
using Optional = std::optional<T>;

struct StrokeStyle {
    Optional<Color> color;
    Optional<double> width;          // points
    Optional<DashPattern> dash;
    Optional<LineCap> cap;
    Optional<LineJoin> join;
};

struct FillStyle {
    Optional<Color> color;
    Optional<double> alpha;
    Optional<HatchPattern> hatch;
};

struct TextStyle {
    Optional<std::string> family;
    Optional<double> size;           // points
    Optional<FontWeight> weight;
    Optional<FontStyle> slant;
    Optional<Color> color;
};

struct Style {
    Optional<StrokeStyle> stroke;
    Optional<FillStyle> fill;
    Optional<TextStyle> text;
    Optional<MarkerStyle> marker;
    Optional<GridStyle> grid;
    // ~30 properties total in v1.0
};

Style cascade(const Style& theme,
              const Style& preset,
              const Style& plot,
              const Style& element);

}
```

`Optional` at every level is the cascade primitive: `nullopt` = "this level does not specify, fall through."

### 3.4 JSON Schema v1.0 (locked: D10.4)

Format example (canonical):
```json
{
  "$schema": "https://lumen.dev/schemas/style/v1.json",
  "lumen_style_version": "1.0",
  "name": "my-paper-theme",
  "extends": "publication",
  "tokens": {
    "color.primary": "#1a73e8",
    "color.background": "#ffffff",
    "font.family": "Inter",
    "font.size.default": 10,
    "line.width.default": 1.5
  },
  "elements": {
    "axis.tick.label": { "font.size": 9 },
    "axis.spine": { "stroke.color": "#333333", "stroke.width": 0.8 },
    "legend.frame": { "stroke": null },
    "grid.major": { "stroke.color": "#eeeeee", "stroke.width": 0.5 }
  }
}
```

**Schema commitments (versioned, breaking-change requires v2.0 + migration tool)**:
- Top-level: `$schema`, `lumen_style_version` (required), `name`, `extends` (optional theme inheritance), `tokens`, `elements`
- Token namespace: dot-separated, free-form within reserved roots (`color.*`, `font.*`, `line.*`, `marker.*`, `grid.*`)
- Element namespace: closed set, defined in schema (`axis.tick.label`, `axis.spine`, `axis.title`, `legend.frame`, `legend.label`, `grid.major`, `grid.minor`, `series.line`, `series.marker`, `series.bar`, etc.)
- All values either literal or token reference (`"$tokens.color.primary"`)

JSON Schema file: `schemas/style-v1.json`, validated in CI on every bundled theme.

### 3.5 Dev tool — "Show style resolution"

Right-click any element in editor → "Inspect style" → modal showing:
Element: series.marker (plot "fig2", trace 1)
─────────────────────────────────────────────
Property              Value      Source
─────────────────────────────────────────────
fill.color            #1a73e8    plot instance
stroke.color          #ffffff    theme (lumen-light)
stroke.width          0.5pt      preset (scatter)
size                  6pt        element override  ← edited
shape                 circle     theme (lumen-light)

This is non-negotiable. Without it, cascade debugging is opaque and Phase 10's UX collapses. Implementation: trace the cascade resolver to record `(property, value, source_level)` triples.

### 3.6 Deliverables
- `src/style/cascade.{h,cpp}` — cascade resolver
- `src/style/types.{h,cpp}` — Style/StrokeStyle/etc structs
- `src/style/json_io.{h,cpp}` — JSON load/save with schema validation
- `schemas/style-v1.json` — JSON Schema definition
- `src/ui/style_inspector.{h,cpp}` — dev tool modal
- `tests/style/test_cascade.cpp` — cascade unit tests (priority, fall-through, full override)
- `tests/style/test_schema_validation.cpp` — schema pass/fail cases
- `tests/style/test_json_roundtrip.cpp` — serialization fidelity
- ADR-061: Cascade architecture (4-level, last-write-wins)
- ADR-062: JSON Schema v1.0 + versioning policy

### 3.7 Exit checklist
- [ ] Cascade resolver passes all 4-level priority tests + nullopt fall-through tests
- [ ] JSON Schema validates all 6 bundled themes (forward-reference to 10.2; placeholder themes acceptable here)
- [ ] Round-trip: `Style → JSON → Style` produces identical Style
- [ ] Style inspector modal opens on right-click, shows resolved values + source level for any element
- [ ] 780 → ~810 tests; all prior tests unchanged

---

## 4. Sub-phase 10.2 — Theme Catalog

### 4.1 Goal
Ship 6 bundled themes covering the major usage contexts. Extend Phase 9.5 vector CI to verify each theme renders consistently.

### 4.2 Theme list (locked: D10.3)

| Theme | Purpose | Key tokens |
|-------|---------|------------|
| `lumen-light` | Apple-mood default, light backgrounds | `color.background: #ffffff`, sans-serif (Inter), thin lines, generous whitespace |
| `lumen-dark` | Dark mode, screen viewing | `color.background: #1a1a1a`, light text, brighter accent palette |
| `publication` | B&W-friendly, journal-neutral | `color.background: #ffffff`, serif (Source Serif), thin black lines, conservative |
| `colorblind-safe` | Wong/Okabe palette across all plot types, viridis-family for scalars | 8-color qualitative palette, sequential = viridis, diverging = cividis |
| `presentation` | Slide-readable | `font.size.default: 18pt`, line widths 2× default, high-contrast palette |
| `print-bw` | Pure grayscale | All series differentiated by line style + marker shape, no color |

### 4.3 Theme files
Location: `resources/themes/{name}.lumen-style.json`. Loaded at app startup, registered in theme registry. User themes load from `~/.lumen/themes/` (Phase 10.4).

### 4.4 Vector CI extension

Phase 9.5 fixture set: 12 plot specs × 1 default theme = 12 reference renders.
Phase 10 extension: 12 plot specs × 6 themes = **72 reference renders** per viewer.

Cost estimate: current vector CI runs ~1m6s for 12 fixtures. 6× fixtures with parallelization across 4 workers ≈ 2–3 min target. If exceeds 5 min, restructure (per-PR runs subset, full matrix on tag-only).

Decision deferred to 10.2 implementation: full matrix every PR vs. theme-rotation (each PR tests a different theme, full matrix on `vphase-*` tags). **Architect to propose; owner to approve.**

### 4.5 Deliverables
- `resources/themes/lumen-light.lumen-style.json`
- `resources/themes/lumen-dark.lumen-style.json`
- `resources/themes/publication.lumen-style.json`
- `resources/themes/colorblind-safe.lumen-style.json`
- `resources/themes/presentation.lumen-style.json`
- `resources/themes/print-bw.lumen-style.json`
- `src/style/theme_registry.{h,cpp}` — built-in theme loading + immutability guard
- `tests/style/test_theme_registry.cpp` — registry tests
- `tests/export/fixtures/source/` — 12 fixtures extended with `theme` parameter
- `tests/export/test_vector_consistency.py` — parameterized over `(fixture, theme, viewer)`
- `tests/export/fixtures/reference/{viewer}/{fixture}_{theme}.png` — 72 references via Git LFS
- ADR-063: Theme catalog selection rationale (why these 6, why not journal-specific)
- ADR-066: Vector CI scaling strategy (matrix vs rotation)

### 4.6 Exit checklist
- [ ] All 6 themes load without schema validation errors
- [ ] All 6 themes render all 12 plot fixtures without crash or visual artifact
- [ ] Vector CI passes for 72 (fixture × theme) × 3 (viewer) = 216 comparisons (or chosen subset per ADR-066)
- [ ] Theme switching at runtime updates plots without recompilation/relaunch
- [ ] Built-in themes cannot be overwritten via API (immutability test)
- [ ] CI runtime increase ≤ 5 min for vector-consistency workflow
- [ ] ~810 → ~835 C++ tests + 60 new pytest reference comparisons

---

## 5. Sub-phase 10.3 — Live Editing + Promotion

### 5.1 Goal
Extend Phase 8 double-click element editing with explicit cascade-promotion controls. User can edit at any cascade level with explicit choice, never silently.

### 5.2 Promotion UI (locked: D10.5)

Color picker (and other element editors) gain a promotion toggle group:
┌─ Edit color: series.marker ────────────────┐
│  Color: [#1a73e8] [picker swatch]          │
│                                            │
│  Apply to:                                 │
│  (•) This element only          [default]  │
│  ( ) All matching in this plot             │
│  ( ) Save to current theme       [warn]    │
│                                            │
│            [Cancel]  [Apply]               │
└────────────────────────────────────────────┘

- **This element only**: writes to element override (cascade priority 4). Phase 8 default behavior.
- **All matching**: writes to plot instance style (priority 3). All elements of same type in this plot inherit.
- **Save to current theme**: writes to active theme (priority 1). Affects every plot using this theme. **Confirmation modal required** with explicit "this changes [N] plots" count + "Save as new theme" alternative.

### 5.3 Built-in theme immutability

If user attempts "Save to current theme" on a built-in theme (`lumen-light`, etc.), system rejects with:
"Built-in themes cannot be modified. Save as new theme?"
[Cancel]  [Save as new theme...]
Forks current theme to user-named copy in `~/.lumen/themes/`, sets active theme to fork.

### 5.4 Undo integration

Every promotion action is a single undo step. Cascade-level metadata preserved in undo stack so undo restores the same level state, not just the value.

### 5.5 Dirty tracking

Theme has `is_modified: bool`. Modified theme + close project → prompt:
"Theme 'my-paper-theme' has unsaved changes."
[Discard]  [Save]  [Save as...]

Built-in themes are never `is_modified` (changes always promote to fork).

### 5.6 Deliverables
- Refactor `src/ui/element_editor.cpp` (Phase 8) to add promotion UI
- `src/ui/promotion_dialog.{h,cpp}` — promotion target picker + confirmations
- `src/style/theme_fork.{h,cpp}` — built-in fork semantics
- Extension to undo stack: `StyleEdit` action with cascade-level metadata
- `tests/ui/test_promotion.cpp` — promotion target writes correct cascade level
- `tests/ui/test_theme_immutability.cpp` — built-in fork enforcement
- `tests/ui/test_undo_promotion.cpp` — undo restores cascade level correctly
- ADR-064: Live promotion semantics
- ADR-065: Built-in theme immutability (fork-only)

### 5.7 Exit checklist
- [ ] Double-click any element → editor with 3 promotion options visible
- [ ] "This element only" writes priority-4 override, verified via style inspector
- [ ] "All matching" writes priority-3 plot style, verified by inspecting siblings
- [ ] "Save to current theme" on user theme writes priority-1, prompts confirmation, undo works
- [ ] "Save to current theme" on built-in theme triggers fork dialog, never modifies built-in
- [ ] Undo of promotion restores correct cascade level (not just value)
- [ ] Dirty tracking prompts on close with unsaved theme changes
- [ ] ~835 → ~860 tests

---

## 6. Sub-phase 10.4 — Style Import/Export UI

### 6.1 Goal
File-level user surface for the cascade system: load themes from disk, save themes, copy styles between elements/plots.

### 6.2 File operations

Menu: **Style → Themes**
- **Load theme from file...** → file picker, validates against schema, registers in user theme registry
- **Export current theme...** → file picker, writes `.lumen-style.json` (resolved with `extends` chain inlined or preserved per user choice)
- **Manage themes...** → list view of built-in + user themes, set active, duplicate, delete (user only), rename (user only)

### 6.3 `extends` resolution

`extends: "publication"` in user theme inherits from built-in. Resolution at load:
final_theme = built_in[publication] ⊕ user_theme  (per-token override)
Token-level merge (not file-level) — user theme overrides only specified tokens, inherits rest.

Cycle detection: `extends` chain validated acyclic at load. Max chain depth: 5 (configurable).

### 6.4 Copy/paste style

Right-click element → **Copy style** → captures element's *resolved* style (or just element override — user choice via submenu).
Right-click another element → **Paste style** → applies as element override on target.

Cross-plot copy/paste supported. Cross-element-type paste (e.g., axis label → series marker) drops incompatible properties silently with toast notification: "3 properties copied, 4 dropped (incompatible)."

### 6.5 Migration stub (forward-looking)

Schema field `lumen_style_version: "1.0"` checked at load. If version is `"1.x"` (future patch), accepted. If `"2.0"` (future major), error: "This file requires a newer Lumen version" — but the **migration framework is scaffolded**: `src/style/migration/v1_to_v2.cpp` empty placeholder + registry pattern. Phase 10 ships v1.0 only, but Phase 11+ can add migrations without architectural change.

### 6.6 Deliverables
- `src/ui/theme_manager_dialog.{h,cpp}` — manage themes UI
- `src/ui/menu/style_menu.cpp` — menu integration
- `src/style/extends_resolver.{h,cpp}` — `extends` chain resolution + cycle detection
- `src/style/clipboard.{h,cpp}` — copy/paste style state
- `src/style/migration/registry.h` — migration framework scaffold (no migrations yet)
- `tests/style/test_extends_resolution.cpp`
- `tests/style/test_extends_cycle_detection.cpp`
- `tests/style/test_copy_paste.cpp`
- `tests/style/test_migration_scaffold.cpp` — verifies v1.0 path + future v2.0 rejection works
- ADR-067: `extends` resolution semantics + cycle policy
- ADR-068: Migration framework design (deferred implementations OK)

### 6.7 Exit checklist
- [ ] Load theme from file → registered + selectable in theme manager
- [ ] Export theme → file produced, validates against schema, round-trips back losslessly
- [ ] `extends` resolves token-level with built-in parents
- [ ] Cycle in `extends` detected with clear error
- [ ] Copy style → paste on same element type → applied as override
- [ ] Cross-element-type paste drops incompatible properties with notification
- [ ] Migration scaffold rejects v2.0 file with clear "newer version required" error
- [ ] ~860 → ~875 tests

---

## 7. Hard rules (verbatim, embedded in T-final)

### 7.1 Review-in-same-commit
At each sub-phase closing, `phase-10.{N}-review.md` and the closing `STATUS.md` entry land in the **same commit**. Stated verbatim in task body. Hard rule held eight phases running (3b/4/5/6/7/8/9/9.5).

### 7.2 Per-deliverable verification in review
The review body contains, for **each user-visible item** in the sub-phase exit checklist, one line: "verified: X works (test/artifact path or command)." Hard rule introduced in Phase 9, enforced through 9.5 (Phase 9.5.1 ADR-path issue surfaced this rule's value — it caught a documentation gap that would otherwise have rotted).

### 7.3 Branch check
Before every commit, run `git branch --show-current`. If output differs from intended branch, stop immediately. Phase 8 stray-branch lesson.

### 7.4 Zero prior regression
All prior tests (780 at Phase 10 start) + every test added in each sub-phase pass unchanged. Single regression blocks the sub-phase.

### 7.5 Umbrella tag convention (new, from Phase 9.5 retrospective)
Each sub-phase gets `vphase-10.N` tag. At Phase 10 close, umbrella tag `vphase-10` points to the same commit as the last sub-phase tag. Going-forward convention for all future phases.

---

## 8. Decisions locked

| ID | Decision | Rationale |
|----|----------|-----------|
| D10.1 | All four style abstractions in scope, unified as 4-level cascade | Theme/preset distinction matches user mental model from Origin/Igor |
| D10.2 | Style fields embedded in plot spec, simple last-write-wins cascade, dev tool mandatory | Per-element override is core to publication-grade requirement |
| D10.3 | 6 bundled themes (lumen-light/dark, publication, colorblind-safe, presentation, print-bw) | Covers major usage contexts; journal-specific themes excluded due to maintenance churn |
| D10.4 | Lumen-native JSON, versioned v1.0, JSON Schema validated | Vega-Lite encoding-centric vs Lumen artifact-centric model mismatch |
| D10.5 | Default per-element override, explicit promotion toggle, built-in immutability | Silent promotion = user surprise; built-in mutation = irreversible damage |

---

## 9. Out of scope (explicitly deferred)

- **CSS file import**: tempting (familiarity) but wrong abstraction (CSS specificity ≠ Lumen cascade). Permanent.
- **Per-axis theming**: e.g., x-axis dark + y-axis light. Edge case, complicates cascade. Deferred to Phase 10.5+ if real demand.
- **Theme transitions/animations**: visual flair, not publication-grade. Out.
- **Theme marketplace / cloud sync**: Phase 16 (Extensibility) territory.
- **OS auto light/dark switch**: Phase 11 (Dashboard) — needs runtime theme observer infrastructure that's better placed there.
- **Style preview thumbnails in theme manager**: nice-to-have, defer to Phase 10.5 if requested.
- **AI-suggested themes / palette generation**: out, no scope.

---

## 10. Risk register

| Risk | Probability | Mitigation |
|------|-------------|------------|
| Cascade scope creep toward CSS specificity | M | ADR-061 explicitly forbids; 10.1 review must verify "no specificity logic" |
| Vector CI runtime explodes with 6× fixtures | H | ADR-066 to choose matrix vs rotation strategy in 10.2; ≤5 min hard cap |
| Phase 8 double-click code refactor breaks existing tests | M | 10.3 starts by snapshotting all Phase 8 tests; refactor preserves all behaviors |
| JSON Schema v1.0 commitment locks future flexibility | M | Migration scaffold in 10.4 prevents architectural rework when v2.0 needed |
| Built-in theme bikeshedding (color choices, font choices) | H | Owner approves token sheet for each theme before 10.2 implementation; no committee |
| Token namespace explosion (every property gets a token) | M | Schema closed-set for elements; tokens free-form but reviewed per theme PR |
| User confusion: "what level is my edit landing on?" | H | Style inspector (3.5) is the single mitigation; usability test before 10.3 close |

---

## 11. Estimates

- Sub-phase 10.1: 7–10 days (foundation, dev tool included)
- Sub-phase 10.2: 4–6 days (theme files cheap, vector CI extension is cost)
- Sub-phase 10.3: 5–7 days (UI care, undo integration is fiddly)
- Sub-phase 10.4: 3–5 days
- Total: ~3–4 weeks calendar
- Test growth projection: 780 → ~875 C++ tests + ~60 new pytest reference comparisons

---

## 12. Dependencies on prior phases

- **Phase 6 (Reactive Engine)**: theme switching at runtime fires reactivity; already supported.
- **Phase 8 (Modern 3D + double-click edit)**: 10.3 refactors the double-click handler. Phase 8 test suite is the regression baseline.
- **Phase 9 (Publication Export + ICC)**: themes must respect ICC color management. Token `color.*` values are sRGB unless theme declares otherwise.
- **Phase 9.5 (Vector CI)**: 10.2 extends the same fixture infrastructure. No new CI tooling needed, only data scaling.

---

## 13. Phase 11 hand-forward

Phase 11 (Dashboard + Linked Views) needs:
- Theme inheritance across plots in same dashboard (already supported by cascade — dashboard = preset level)
- Cross-plot style copy (already supported by 10.4)
- Per-plot theme override within a dashboard (cascade priority 3 handles this)

Phase 10 should land all three without additional Phase 11 work needed in style domain.

