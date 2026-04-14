# ADR-055: Text-as-path in publication export

## Status
Proposed (Phase 9.5.1)

## Context
Cross-viewer font rendering differences make pixel-level SVG/PDF
consistency impossible when text is exported as `<text>` elements
(SVG) or embedded font glyphs (PDF). FreeType, Pango, CoreText,
and DirectWrite all produce different hinting, kerning, and
anti-aliasing for the same font at the same size.

Phase 9.5.1 needs a vector consistency CI gate that compares
renderings across Chromium, pdftocairo, and Inkscape. If text
is rendered as text, the gate will never pass.

## Decision
All text in SVG/PDF export is converted to outline paths via
`QPainterPath::addText()`. PlotRenderer gains a `textAsPath`
mode (default true for SVG/PDF export, false for screen
rendering).

In textAsPath mode, every `painter.drawText()` call is replaced
with:
```cpp
QPainterPath path;
path.addText(x, y, font, text);
painter.drawPath(path);
```

This produces `<path d="...">` in SVG and glyph outlines in PDF.

### Trade-off
- **Lost**: selectable/searchable text in exported files.
- **Gained**: pixel-identical rendering across all viewers.

This trade-off is acceptable for publication-grade figures (the
primary use case). Selectable text will be offered as an opt-in
in Phase 10 via `export.text_as_path: bool` configuration.

## Consequences
- + Cross-viewer consistency achievable (same path data everywhere)
- + Eliminates font embedding complexity (no font needed in output)
- + Vector consistency CI gate can use tight thresholds
- - SVG file size increases 3-10x (path data vs text references)
- - Text not selectable/searchable in output files
- - Screen rendering unchanged (textAsPath=false for QWidget paint)

## Alternatives considered
- **Keep text as text, loosen CI thresholds**: Rejected. SSIM < 0.9
  for text differences — too loose to catch real regressions.
- **Font subsetting + forced metrics**: Rejected. Even with
  identical fonts, hinting algorithms differ across renderers.
  Pixel differences persist.
- **Raster-only export**: Rejected. Users need vector SVG/PDF for
  journal submissions. Raster is already available (PNG export).
