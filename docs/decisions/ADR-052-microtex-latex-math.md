# ADR-052: MicroTeX for LaTeX math rendering

## Status
Proposed (Phase 9)

## Context
Scientific figures need math notation in axis labels, titles,
and annotations (e.g., σ², ∫₀^∞, V_m (mV)). External LaTeX
processes (pdflatex) require a full TeX installation. WebEngine
(for KaTeX/MathJax) drags in Chromium. MicroTeX is a lightweight
C++ library that renders LaTeX math to bitmap or vector paths.

## Decision
Vendor MicroTeX via CMake FetchContent as a static library.
MicroTeX (MIT license, NanoMichael) renders TeX math formulas
without external dependencies.

### MathRenderer API
- `render(latex, pointSize, color)` → QImage (for raster export)
- `renderToPath(latex, pointSize)` → QPainterPath (for vector
  SVG/PDF export)
- `isValidLatex(latex)` → bool (no crash on malformed input)

### Supported macro coverage
MicroTeX supports standard LaTeX math: Greek letters, super/
subscripts, fractions, integrals, summations, matrices, sqrt,
text, mathrm, mathbf, common symbols. Coverage statement
documented in export/CLAUDE.md.

### Unsupported syntax fallback
If MicroTeX cannot parse a string, isValidLatex() returns false.
The UI shows the raw text with a warning icon. No crash, no
blank output.

### Font
MicroTeX bundles Computer Modern math fonts. These are compiled
into the static library. No additional font files needed for
math rendering.

### Integration points
- AxisDialog: "LaTeX" checkbox → label rendered via MathRenderer
- TitleDialog: "LaTeX" checkbox → title rendered via MathRenderer
- Legend: per-entry LaTeX flag
- TextAnnotation: inline $...$ triggers MathRenderer for the
  delimited substring

## Consequences
- + Native C++ rendering, no external process
- + MIT license, no compatibility issues
- + QPainterPath output gives true vector math in SVG/PDF
- + Computer Modern fonts bundled automatically
- + ~30s CI build time increase
- - MicroTeX does not cover all of LaTeX (no TikZ, no
  environments beyond math, limited text-mode). Acceptable:
  axis labels and annotations need math, not documents.
- - Adds ~500KB to binary (MicroTeX + CM fonts)
- - Build system requires disabling MicroTeX's optional Cairo/
  GTK backends (MICROTEX_NO_GRAPHIC=ON)

## Alternatives considered
- **KaTeX via QWebEngine**: Rejected. Drags in Chromium (~100MB).
  Overkill for math formulas. Platform-specific issues.
- **External pdflatex**: Rejected. Requires user to install a
  full TeX distribution. Runtime dependency violates Lumen's
  single-binary goal.
- **MathJax via JS engine**: Rejected. Requires embedding a
  JavaScript engine. Heavy, slow startup.
- **No math support**: Rejected. Explicit requirement from spec.
  Scientific figures need math notation.
- **Unicode math only** (render Unicode math symbols via Qt):
  Rejected. No fractions, no integrals, no subscript layout.
  Not publication-grade.
