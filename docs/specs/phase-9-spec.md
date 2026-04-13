# Phase 9 — Publication-Grade Export

## Goal

Transform Phase 4's basic PNG/SVG/PDF export into true
publication-grade output. ICC color management for print
accuracy, font subset embedding with user-selectable academic
fonts, automated cross-viewer vector consistency, native LaTeX
math rendering, full annotation layer (arrows, boxes, callouts,
scale bar, color bar, free-positioned labels), and
non-blocking export with cancel — together turning Lumen into
a tool that produces manuscript-ready figures without
post-processing in Illustrator or Inkscape.

This phase resolves MATLAB figure limitations #4 (export quality
inconsistency) and #12 (primitive annotation), and absorbs what
was originally planned for Phase 12. After Phase 9, the v2
roadmap reduces to: Phase 10 (Style System), Phase 11
(Dashboard + Linked Views, including inset), Phase 13
(Computational Integration), Phase 14 (Reproducibility), Phase
15 (Non-Cartesian), Phase 16 (Plugins). Phase 12 narrative-only
work folds into Phase 14.

## Design decisions (approved)

1. **Color management**: ICC profile-based. User-selectable
   profile per export.
2. **Fonts**: subset embedding + user-selectable academic font
   (Computer Modern, Times, Helvetica, Source Serif Pro)
3. **Vector consistency**: round-trip CI verification across
   Chrome, Firefox, Inkscape, headless rendering for SVG;
   pdftocairo for PDF
4. **LaTeX math**: native MicroTeX integration (no external
   process, no WebEngine)
5. **Annotation layer**: full — arrows, boxes, callouts, free
   text labels, inline math, scale bar, color bar
6. **Inset**: deferred to Phase 11
7. **Export pipeline**: background thread with cancel button +
   progress bar

## Active agents

Architect, Backend, Frontend, QA, Integration, Docs.

## Scope: six internal sub-phases

- **9.1** ICC color management infrastructure
- **9.2** Font system (subset embedding, academic font picker)
- **9.3** Cross-viewer vector consistency (round-trip CI tests)
- **9.4** MicroTeX integration for axis labels, titles, annotations
- **9.5** Annotation layer (arrows, boxes, callouts, scale bar,
  color bar, text labels with positioning)
- **9.6** Async export pipeline with progress + cancel

Each sub-phase ends with a verification gate.

## Phase 9.1 — ICC Color Management

### export/ColorProfile.{h,cpp}
namespace lumen::export {
class ColorProfile {
public:
enum class Builtin {
sRGB,                  // screen default
AdobeRGB,
DisplayP3,
CMYK_USWebCoatedSWOP,  // common print
CMYK_FOGRA39,           // European print
Gray_Gamma22
};
static ColorProfile builtin(Builtin id);
static ColorProfile fromIccFile(const QString& path);

QByteArray iccBytes() const;     // for embedding in PDF/PNG
QString name() const;
bool isCMYK() const;
bool isRGB() const;
bool isGrayscale() const;

QColor convert(QColor source, const ColorProfile& target) const;
};
}

Implementation: bundled with LittleCMS (lcms2) as static lib.
Built-in profiles compiled in. User ICC files parsed via
lcms2.

### export/ColorPipeline.{h,cpp}

Wraps PlotRenderer to convert all colors through ColorProfile
during export. PNG/PDF embed the profile chunk (PNG iCCP chunk,
PDF /ICCBased ColorSpace).

### Tests 9.1

- test_color_profile_builtin.cpp: each builtin profile loads
  and produces valid ICC bytes
- test_color_profile_from_file.cpp: standard ICC file (e.g.
  sRGB IEC61966-2.1) parses
- test_color_conversion.cpp: known sRGB → CMYK conversion
  matches reference values within tolerance
- test_png_iccp_chunk.cpp: exported PNG contains iCCP chunk
- test_pdf_iccbased_colorspace.cpp: exported PDF declares
  /ICCBased

Gate M9.1: ICC profiles work, embedded in PNG and PDF.

## Phase 9.2 — Font System

### export/FontEmbedder.{h,cpp}
class FontEmbedder {
public:
void registerFont(const QString& family, const QByteArray& fontData);
QByteArray buildSubset(const QString& family,
const QSet<QChar>& usedGlyphs) const;
void embedInPdf(QPdfWriter* writer);
void embedInSvg(QXmlStreamWriter& writer);
};

Subset using FreeType + custom subsetter, OR bundled HarfBuzz
subset.

### Bundled academic fonts

Phase 9 ships with these fonts (open-licensed):
- **Computer Modern** (LaTeX default, public domain)
- **Times New Roman / Liberation Serif** (sans-cost alternative)
- **Helvetica / Liberation Sans**
- **Source Serif Pro** (Adobe SIL OFL)

User can also load arbitrary OTF/TTF.

### ui/FontPicker.{h,cpp}

Combo box in ExportDialog with:
- Built-in academic fonts (above)
- "System..." opens font dialog
- Preview shows axis labels in selected font

### Tests 9.2

- test_font_embedder_subset.cpp: subset of "abc" produces
  smaller bytes than full font
- test_pdf_font_embed.cpp: exported PDF contains embedded font
  table with subset
- test_svg_font_embed.cpp: exported SVG contains @font-face
  with base64-encoded subset
- test_academic_fonts_loaded.cpp: 4 built-in fonts available

Gate M9.2: PDF/SVG with embedded fonts opens identically on a
machine without those fonts installed (CI test).

## Phase 9.3 — Cross-Viewer Vector Consistency

### CI infrastructure

New CI job: vector-consistency.

For each canonical test plot:
1. Export to SVG and PDF
2. Render SVG via headless Chrome (puppeteer or playwright),
   Firefox (geckodriver), and Inkscape CLI
3. Render PDF via pdftocairo and pdftoppm
4. Compute pixel difference (SSIM or PSNR) between renderings
5. Fail if difference exceeds threshold (PSNR > 40 dB target)

### Canonical test plots

- Simple line plot
- Scatter with markers
- Bar chart
- Heatmap with colormap
- Plot with LaTeX axis labels (Phase 9.4 dependency)
- Plot with annotations (Phase 9.5 dependency)
- All plot types with each built-in font

### Tests 9.3

- test_svg_strict_compliance.cpp: exported SVG validates against
  SVG 1.1 schema
- test_pdf_pdfa_compliance.cpp: exported PDF passes basic PDF/A
  checks (veraPDF or similar)
- CI integration test: vector-consistency job

Gate M9.3: vector-consistency CI job green for all canonical
test plots.

## Phase 9.4 — LaTeX Math via MicroTeX

### export/MathRenderer.{h,cpp}

Wraps MicroTeX (statically linked).
class MathRenderer {
public:
static QImage render(const QString& latex,
double pointSize,
QColor color);
// For SVG/PDF export: render to vector path
static QPainterPath renderToPath(const QString& latex,
                                  double pointSize);

static bool isValidLatex(const QString& latex);
};

### Integration points

- **Axis labels**: existing AxisDialog gains "LaTeX mode"
  toggle. When on, label string is rendered via MathRenderer.
- **Title**: TitleDialog gains same toggle.
- **Legend entries**: each series name supports LaTeX.
- **Annotations** (Phase 9.5): inline LaTeX in text labels via
  `$...$` syntax.

### Tests 9.4

- test_microtex_basic.cpp: `\sigma^2` renders correctly
- test_microtex_complex.cpp: matrix, integral, fraction render
- test_microtex_invalid.cpp: malformed LaTeX returns
  isValidLatex=false, doesn't crash
- test_axis_label_latex.cpp: AxisDialog with LaTeX mode
  produces vector output in SVG/PDF
- test_legend_latex.cpp: legend entry with LaTeX renders

Gate M9.4: LaTeX in axis labels, titles, legend works in
PNG/SVG/PDF export.

## Phase 9.5 — Annotation Layer

### plot/AnnotationLayer.{h,cpp}
class AnnotationLayer {
public:
void addAnnotation(std::unique_ptr<Annotation>);
void removeAnnotation(int id);
QList<Annotation*> all() const;
void paint(QPainter*, const CoordinateMapper&,
           const QRectF& plotArea) const;
std::optional<HitResult> hitTest(QPoint pixel) const;
};

### Annotation types (each is a class)
class Annotation {
public:
enum class Type { Arrow, Box, Callout, Text, ScaleBar, ColorBar };
enum class Anchor { Data, Pixel, AxisFraction };
virtual Type type() const = 0;
virtual void paint(QPainter*, const CoordinateMapper&) const = 0;
virtual QRectF boundingRect() const = 0;
virtual QJsonObject toJson() const = 0;
};

#### ArrowAnnotation
From point A to point B. Arrowhead style (open, filled,
double). Color, line width.

#### BoxAnnotation
Rectangle. Fill color, border color, border width, dash style.

#### CalloutAnnotation
Box + text + arrow pointing to a data point. Text supports
inline LaTeX.

#### TextAnnotation
Text at a position. Anchor: Data (data coordinates), Pixel
(absolute pixel), AxisFraction (0..1 of plot area). Inline
LaTeX support via `$...$` parsing → MathRenderer paths.
Rotation angle.

#### ScaleBar
Horizontal bar with length label, configured by data units.
Common in microscopy figures. Auto-positioned in corner by
default, draggable.

#### ColorBar
For Heatmap and any colormapped PlotItem. Vertical or
horizontal. Tick labels with units.

### ui/AnnotationToolbar.{h,cpp}

Floating toolbar on PlotCanvas with buttons: Arrow, Box,
Callout, Text, ScaleBar, ColorBar. Click → enter placement
mode → click on plot to place.

### ui/AnnotationPropertyDialog.{h,cpp}

Per-annotation property dialog (double-click annotation to
open). Same paradigm as Phase 3a/3b/8.

### Workspace serialization

WorkspaceFile gains "annotations" array per plot. Each
annotation serializes via its toJson().

### Tests 9.5

- test_annotation_layer.cpp: add, remove, hit-test
- test_arrow_annotation.cpp: paint, bounds, JSON roundtrip
- test_box_annotation.cpp: same
- test_callout_annotation.cpp: with arrow + text + box
- test_text_annotation.cpp: 3 anchor modes, rotation, inline
  LaTeX
- test_scale_bar.cpp: data unit calibration, auto-position
- test_color_bar.cpp: maps to Heatmap colormap, units shown
- test_annotation_toolbar.cpp: toolbar interactions
- test_annotation_dialog.cpp: property edits via CommandBus
- test_annotation_workspace.cpp: roundtrip with all 6 types

Gate M9.5: human creates a publication-style figure with
multiple annotations and exports to PDF.

## Phase 9.6 — Async Export Pipeline

### export/ExportTask.{h,cpp}
class ExportTask : public QObject {
Q_OBJECT
public:
ExportTask(const ExportOptions& opts, QObject* parent = nullptr);
void start();
void cancel();

bool isRunning() const;
int progressPercent() const;
signals:
void progress(int percent);
void finished(bool success, const QString& outputPath);
void error(const QString& message);
private:
QThread* m_thread;
std::atomic<bool> m_cancelRequested;
};

Export runs in QThread. Main thread receives progress and
finished signals. Cancel is cooperative — render loop checks
m_cancelRequested between primitives.

### ui/ExportProgressDialog.{h,cpp}

Modal-ish dialog: progress bar, current step ("Rendering
plot..." → "Embedding fonts..." → "Writing file..."), Cancel
button.

### Tests 9.6

- test_export_task_progress.cpp: signals emitted in order
- test_export_task_cancel.cpp: cancel mid-render produces no
  output file, no crash
- test_export_progress_dialog.cpp: GUI integration

Gate M9.6: large export (4K PNG, 600 DPI) shows progress and
can be cancelled.

## ADRs

- ADR-049 ICC color management via lcms2. Alternatives:
  no color management (rejected — wrong colors in print),
  sRGB-only (rejected — print needs CMYK), platform color
  systems (rejected — non-portable)
- ADR-050 Font subset embedding strategy. Alternatives: no
  embedding (rejected — fonts missing on other systems), full
  embedding (rejected — file size), reference fonts by name
  (rejected — same problem as no embedding)
- ADR-051 Cross-viewer vector consistency CI. Alternatives:
  spec compliance only (rejected — viewers ignore spec),
  manual testing (rejected — not scalable)
- ADR-052 MicroTeX for LaTeX math. Alternatives: KaTeX via
  WebEngine (rejected — heavy Chromium dependency), external
  pdflatex process (rejected — runtime dependency), MathJax
  (rejected — JS engine), no math support (rejected —
  requirement)
- ADR-053 Annotation layer in plot/ module. Alternatives:
  separate annotation/ module (rejected — annotations are
  rendered alongside PlotItems, same coordinate system),
  per-PlotItem annotations (rejected — annotations span plot
  area, not bound to one series)
- ADR-054 Async export with QThread + cooperative cancel.
  Alternatives: process-based (rejected — overhead), preemptive
  cancel (rejected — Qt rendering not preemptible)

## Architecture updates

docs/architecture.md "Phase 9 additions":
- New export/ submodule: ColorProfile, ColorPipeline,
  FontEmbedder, MathRenderer, ExportTask
- New plot/AnnotationLayer + 6 Annotation subclasses
- ExportDialog (Phase 4) extended with: color profile picker,
  font picker, async + progress
- AxisDialog, TitleDialog, LegendDialog gain LaTeX mode toggle
- Plot data flow with annotations:
  PlotCanvas paints PlotItems → AnnotationLayer paints on top
- Workspace format: annotations array per plot
- CI: new vector-consistency job
- Bundled libs: lcms2 (static), MicroTeX (static), 4 fonts

## Acceptance criteria

Manual flow per sub-phase as per M-gates above.

Regression at every M-gate:
- [ ] All 700 Phase 8 tests pass unchanged
- [ ] Phase 4 PNG/SVG/PDF export still works (now with default
      sRGB / system font / synchronous mode for compatibility)
- [ ] CI green on Ubuntu Debug/Release + macOS Debug/Release
- [ ] New vector-consistency CI job green

## Real-data exit criterion

Human creates a complete publication figure including:
- [ ] Real data plot (from electrophysiology, microscopy, or
      simulation)
- [ ] LaTeX axis labels and title (e.g. `V_m (\mathrm{mV})`)
- [ ] Annotation: at least 2 arrows, 1 callout with LaTeX,
      1 scale bar
- [ ] Color bar (if Heatmap or colormapped data)
- [ ] Export as PDF with CMYK profile (or sRGB if screen
      target)
- [ ] PDF opens identically in Adobe Reader, Preview, and
      Chrome
- [ ] PDF imports cleanly into LaTeX manuscript via
      \includegraphics
- [ ] Async export with cancel works for at least one large
      figure

## Non-goals (deferred or absorbed)

- Inset (figure-in-figure): Phase 11
- Style presets / saved styles: Phase 10
- Multi-plot dashboard: Phase 11
- Reproducibility / command history export: Phase 14
- Plugin annotations: Phase 16
- Animation export (video): not planned
- 3D figure annotations beyond what 2D supports: Phase 9.5
  covers 2D only; 3D annotations are a future extension
- PDF/X (commercial print) compliance beyond PDF/A

## Risks and mitigations

| Risk | Mitigation |
|---|---|
| lcms2 build on macOS | Static link, test in CI from Phase 9.1 start |
| MicroTeX font dependencies | Bundle Computer Modern with MicroTeX statically |
| Cross-viewer SVG differences exceed threshold | Iterative tightening of generated SVG; document known limitations |
| QThread + Qt rendering thread safety | Render to QImage off-screen in worker; only file I/O on main |
| Cancel during PDF write produces corrupt file | Write to temp file, atomic rename on success only |
| ICC profile sizes inflate PNG/PDF | Subset profile or reference vs embed; document trade-off |
| MicroTeX LaTeX subset coverage | Document supported macros; degrade gracefully on unsupported |
| Annotation count slows rendering | Spatial index for hit-test; lazy paint outside viewport |

## Task breakdown

### Architect (S)
- phase-9-plan.md with 6 sub-phase structure and M-gates
- ADR-049 through ADR-054
- Architecture update
- STATUS opening entry

### Backend (XL)
- 9.1: ColorProfile, ColorPipeline, lcms2 integration
- 9.2: FontEmbedder, font subset
- 9.4: MathRenderer (MicroTeX wrapper)
- 9.5: AnnotationLayer + 6 Annotation classes + commands
- 9.6: ExportTask (QThread)

### Frontend (XL)
- 9.1: ExportDialog color profile picker
- 9.2: FontPicker UI, font preview
- 9.4: LaTeX mode toggles in AxisDialog/TitleDialog/LegendDialog
- 9.5: AnnotationToolbar, AnnotationPropertyDialog (per type)
- 9.6: ExportProgressDialog

### QA (L)
- Per-sub-phase unit tests
- Vector-consistency CI job (Phase 9.3)
- Regression: 700 Phase 8 tests unchanged

### Integration (S)
- Sub-phase merge windows
- vphase-9 tag

### Docs (S)
- README update (publication-grade export)
- export/CLAUDE.md (new submodule)
- plot/CLAUDE.md update (AnnotationLayer)
- phase-9-review.md WRITTEN AND COMMITTED IN THE SAME COMMIT
  AS THE CLOSING .lumen-ops/STATUS.md ENTRY. Verbatim in T-final
  task body. Phase 3b/4/5/6/7/8 proved this discipline.
- **NEW review rule (Phase 8 lesson)**: review must verify that
  every exit checklist item ACTUALLY WORKS, not just that
  tests pass. Specifically: human or QA confirms each
  user-visible deliverable with a one-line note in the review
  ("verified: ICC PDF opens in Preview with correct colors").

## Exit checklist

Phase 9.1 ICC:
- [ ] Built-in profiles load
- [ ] User ICC files parse
- [ ] PNG iCCP chunk embedded
- [ ] PDF /ICCBased declared

Phase 9.2 Fonts:
- [ ] 4 academic fonts bundled and loadable
- [ ] PDF/SVG with embedded fonts opens on font-less machine

Phase 9.3 Vector consistency:
- [ ] vector-consistency CI job green
- [ ] PSNR > 40 dB across viewers for canonical plots

Phase 9.4 LaTeX:
- [ ] MicroTeX renders standard math
- [ ] LaTeX in axis labels, titles, legend exports correctly

Phase 9.5 Annotation:
- [ ] All 6 annotation types work
- [ ] Toolbar functional
- [ ] Property dialogs work via CommandBus
- [ ] Workspace roundtrip preserves annotations

Phase 9.6 Async export:
- [ ] Progress shown for large exports
- [ ] Cancel produces no output, no crash

Phase 9 overall:
- [ ] Build clean on Ubuntu and macOS, Debug + Release
- [ ] All 700 Phase 8 tests pass unchanged
- [ ] New Phase 9 tests pass (target 700 → 850+)
- [ ] CI green on all 4 platforms + vector-consistency job
- [ ] Real-data exit criterion passed (publication figure)
- [ ] ADR-049 through ADR-054 committed
- [ ] phase-9-review.md committed IN SAME COMMIT as closing
      STATUS entry, with per-deliverable verification notes
- [ ] vphase-9 tag pushed
