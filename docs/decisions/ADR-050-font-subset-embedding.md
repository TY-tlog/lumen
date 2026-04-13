# ADR-050: Font subset embedding strategy

## Status
Proposed (Phase 9)

## Context
Exported PDF/SVG must display identical text on machines that
lack the fonts used during export. Full font embedding inflates
file size (a single TTF can be 500KB-2MB). Subset embedding
includes only the glyphs actually used.

## Decision
Use HarfBuzz's hb_subset API for font subsetting. HarfBuzz is
already a transitive dependency of Qt on most platforms, but we
vendor it via FetchContent to ensure consistent behavior.

FontEmbedder:
1. Receives full TTF/OTF binary data via registerFont().
2. On export, collects all QChars rendered in the plot.
3. Calls hb_subset_input_create(), adds used codepoints,
   calls hb_subset() to produce a minimal font binary.
4. PDF: subset TTF embedded as /FontFile2 stream.
5. SVG: subset base64-encoded in @font-face declaration.

### Bundled academic fonts (all open-licensed)
- **Computer Modern** (public domain, Donald Knuth)
- **Liberation Serif** (SIL OFL, Red Hat — Times substitute)
- **Liberation Sans** (SIL OFL, Red Hat — Helvetica substitute)
- **Source Serif Pro** (SIL OFL, Adobe)

Fonts stored as binary files in third_party/fonts/ (~8 MB total).
Compiled into binary via Qt resource system.

## Consequences
- + Minimal file size (subset typically 5-20KB vs 500KB full)
- + PDF/SVG portable across machines
- + HarfBuzz is mature, actively maintained
- + Open-licensed fonts avoid trademark issues (Liberation
  instead of Times New Roman / Helvetica)
- - HarfBuzz adds ~1MB to binary and ~30s to CI build
- - Subset correctness depends on complete glyph collection
  (must track all rendered text, including tick labels)

## Alternatives considered
- **No embedding** (reference fonts by name): Rejected. Font
  missing on target machine → fallback font → layout shifts.
  Defeats publication-grade goal.
- **Full font embedding**: Rejected. 500KB-2MB per font per
  export. Subset is strictly better.
- **FreeType custom subsetter**: Rejected. FreeType can read
  fonts but has no built-in subset API. HarfBuzz hb_subset is
  purpose-built and handles edge cases (composite glyphs,
  ligature tables, etc.).
