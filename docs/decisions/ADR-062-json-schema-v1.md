# ADR-062: JSON Schema v1.0 + versioning policy

## Status
Proposed (Phase 10.1)

## Context
Style files need a stable, versioned format that users can edit
by hand, validate automatically, and share across machines. The
format must support future evolution without breaking existing
files.

## Decision
Lumen-native JSON format with JSON Schema validation and semantic
versioning.

### Schema structure
```json
{
  "$schema": "https://lumen.dev/schemas/style/v1.json",
  "lumen_style_version": "1.0",
  "name": "theme-name",
  "extends": "parent-theme-name",
  "tokens": { "color.primary": "#1a73e8", ... },
  "elements": { "axis.tick.label": { "font.size": 9 }, ... }
}
```

### Required fields
- `lumen_style_version`: string, required. Format: "major.minor".
- All other fields optional (empty style is valid).

### Token namespace (reserved roots)
- `color.*` — all color tokens
- `font.*` — font family, size, weight
- `line.*` — line width, dash patterns
- `marker.*` — marker size, shape
- `grid.*` — grid visibility, colors
- `spacing.*` — reserved for future

Convention: `{root}.{qualifier}.{detail}` (dot-separated).
User tokens may use any root but reserved roots have semantic
meaning for the cascade resolver.

### Element namespace (closed set)
Elements are named by their position in the plot hierarchy:
- `axis.tick.label`, `axis.spine`, `axis.title`
- `legend.frame`, `legend.label`
- `grid.major`, `grid.minor`
- `series.line`, `series.marker`, `series.bar`, `series.fill`
- `title`, `background`
- `annotation.text`, `annotation.arrow`, `annotation.box`

New elements added in future versions; existing elements never
removed or renamed (backward compat).

### Versioning policy
- **Patch** (1.0 → 1.1): additive (new optional fields, new
  elements). Old files still valid. No migration needed.
- **Major** (1.x → 2.0): breaking change (field rename, semantic
  change). Requires migration tool. Phase 10.4 scaffolds the
  migration framework.

### Token references
Values can reference tokens: `"$tokens.color.primary"`.
Resolved at load time. Unresolved references produce a warning
and fall through to default.

### Validation
JSON Schema file at `schemas/style-v1.json` validated in CI on
every bundled theme file. User theme files validated at load with
clear error messages.

## Consequences
- + Human-readable and hand-editable
- + JSON Schema enables tooling (editors, linters)
- + Versioning prevents silent breakage
- + Token references enable DRY themes
- - JSON is verbose (mitigated: style files are small, < 5 KB)
- - No comments in JSON (mitigated: use `_comment` fields)
- - Schema must be maintained alongside implementation

## Alternatives considered
- **CSS/SCSS**: Rejected. CSS specificity model is wrong for
  Lumen's 4-level cascade. SCSS adds a build step.
- **YAML**: Rejected. YAML parsing is more complex, whitespace-
  sensitive, and Qt has no built-in YAML support (vs QJsonDocument).
- **Vega-Lite encoding**: Rejected. Vega-Lite's encoding-centric
  model doesn't map to Lumen's artifact-centric style system.
- **Binary format**: Rejected. Not human-editable. Style files
  are small enough that JSON overhead is negligible.
