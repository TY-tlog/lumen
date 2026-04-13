# Phase 9 Plan — Publication-Grade Export

**Spec**: `docs/specs/phase-9-spec.md`
**Architect**: Claude (Architect agent)
**Date**: 2026-04-13

---

## Overview

Six sub-phases, 26 tasks + T-final closing. Each sub-phase ends
with a verification gate (M9.x). 700-test regression gate at
every M-point. CI green on 4 platforms. Review-in-same-commit
with per-deliverable verification notes (Phase 8 lesson).

## Hard constraints

1. **Review-in-same-commit + per-deliverable verification**:
   phase-9-review.md committed in same commit as closing STATUS
   entry. Review includes a one-line verification note for each
   user-visible exit checklist item. Verbatim in T-final body.
2. **700-test regression gate** at every M-point (M9.1-M9.6).
3. **CI green on 4 platforms + vector-consistency job** (from M9.3).
4. **Stray branch prevention**: every git operation preceded by
   `git branch --show-current` verification.
5. **Spec design decisions are approved**: 7 decisions in spec
   §"Design decisions" can be refined but not revised.

## Critical path

```
T1 (lcms2 vendor) ─→ T2 ─→ T3 ─→ T4 ─→ M9.1
T5 (fonts) ──────────→ T6 ─→ T7 ─→ T8 ─→ M9.2
                               T9 ─→ T10 ─→ T11 ─→ M9.3
T12 (MicroTeX) ──────→ T13 ─→ T14 ─→ T15 ─→ M9.4
                         T16 ─→ T17 ─→ T18 ┐
                                   T19 ─→ T20 ├→ T21 ─→ T22 ─→ M9.5
T23 (async) ──────────→ T24 ─→ T25 ─→ M9.6
T26 (docs + close)
```

T1, T5, T12, T23 are independent — can start in parallel.
T10 (vector CI) blocks all subsequent merges from M9.3 on.
T16 (AnnotationLayer base) blocks T17-T22.

---

## Sub-phase 9.1 — ICC Color Management

### T1: lcms2 vendor + ColorProfile + builtins
- **Owner**: Backend
- **Files**: CMakeLists.txt (root + cmake/), third_party/lcms2/,
  src/lumen/export/ColorProfile.{h,cpp},
  src/lumen/export/CMakeLists.txt (NEW module)
- **Acceptance**: lcms2 builds as static lib on Ubuntu+macOS.
  ColorProfile::builtin(sRGB) returns valid ICC bytes.
  ColorProfile::fromIccFile() parses standard ICC file.
  ColorProfile::convert() converts sRGB→CMYK within tolerance.
- **Size**: L
- **Precondition**: `git branch --show-current` before first commit.

### T2: ColorPipeline + PNG iCCP / PDF /ICCBased
- **Owner**: Backend
- **Files**: src/lumen/export/ColorPipeline.{h,cpp},
  src/lumen/core/io/FigureExporter.{h,cpp} (extended)
- **Acceptance**: ColorPipeline wraps PlotRenderer, converts colors
  through profile. Exported PNG contains iCCP chunk (verified by
  reading raw bytes). Exported PDF declares /ICCBased ColorSpace
  (verified by grep on PDF text).
- **Size**: L

### T3: ExportDialog color profile picker
- **Owner**: Frontend
- **Files**: src/lumen/ui/ExportDialog.{h,cpp} (extended)
- **Acceptance**: Combo box lists built-in profiles + "Custom..."
  option. Selected profile passed to FigureExporter::Options.
- **Size**: S

### T4: ICC tests
- **Owner**: QA
- **Files**: tests/unit/test_color_profile_builtin.cpp,
  test_color_profile_from_file.cpp, test_color_conversion.cpp,
  test_png_iccp_chunk.cpp, test_pdf_iccbased_colorspace.cpp
- **Acceptance**: 5 test files, all pass. 700 Phase 8 tests unchanged.
- **Size**: M

### Gate M9.1
- [ ] All 700 Phase 8 tests pass
- [ ] 5 new ICC tests pass
- [ ] CI green on 4 platforms
- [ ] Built-in profiles load
- [ ] PNG iCCP chunk embedded
- [ ] PDF /ICCBased declared

---

## Sub-phase 9.2 — Font System

### T5: FontEmbedder + subset implementation
- **Owner**: Backend
- **Files**: src/lumen/export/FontEmbedder.{h,cpp}
- **Acceptance**: registerFont() accepts TTF/OTF binary data.
  buildSubset() for "abc" produces smaller output than full font.
  embedInPdf() and embedInSvg() produce valid embedded font data.
- **Size**: L

### T6: Academic font integration (4 fonts vendored)
- **Owner**: Backend
- **Files**: third_party/fonts/ (Computer Modern, Liberation Serif,
  Liberation Sans, Source Serif Pro), src/lumen/export/FontEmbedder.cpp
  (registers built-ins at startup)
- **Acceptance**: 4 fonts loadable via FontEmbedder.
  All fonts open-licensed (verified: CM=public domain,
  Liberation=OFL, Source Serif=OFL).
- **Size**: S

### T7: FontPicker UI with preview
- **Owner**: Frontend
- **Files**: src/lumen/ui/FontPicker.{h,cpp} (NEW),
  src/lumen/ui/ExportDialog.{h,cpp} (extended)
- **Acceptance**: Combo lists 4 built-in fonts + "System..." option.
  Preview renders axis label text in selected font. Selected font
  passed to FigureExporter::Options.
- **Size**: M

### T8: Font tests
- **Owner**: QA
- **Files**: tests/unit/test_font_embedder_subset.cpp,
  test_pdf_font_embed.cpp, test_svg_font_embed.cpp,
  test_academic_fonts_loaded.cpp
- **Acceptance**: 4 test files pass. 700 Phase 8 tests unchanged.
- **Size**: M

### Gate M9.2
- [ ] All Phase 8 + 9.1 tests pass
- [ ] 4 new font tests pass
- [ ] CI green on 4 platforms
- [ ] 4 academic fonts bundled
- [ ] PDF with embedded font opens on font-less machine

---

## Sub-phase 9.3 — Cross-Viewer Vector Consistency

### T9: SVG strict compliance + PDF/A basics
- **Owner**: Backend
- **Files**: src/lumen/core/io/FigureExporter.{h,cpp} (SVG+PDF
  improvements)
- **Acceptance**: Exported SVG validates against SVG 1.1 schema
  (xmllint). Exported PDF passes basic structure checks.
- **Size**: M

### T10: vector-consistency CI job
- **Owner**: Integration
- **Files**: .github/workflows/vector-consistency.yml (NEW),
  scripts/vector-consistency-check.sh (NEW),
  tests/fixtures/vector-consistency/ (canonical reference PNGs)
- **Acceptance**: CI job runs on Ubuntu, renders canonical plots via
  Playwright (Chrome+Firefox), Inkscape CLI, pdftocairo. Computes
  PSNR. Job passes with >40 dB for all canonical plots. First 2
  weeks: warn-only. After: merge-blocking.
- **Size**: L

### T11: SVG/PDF compliance tests
- **Owner**: QA
- **Files**: tests/unit/test_svg_strict_compliance.cpp,
  test_pdf_structure.cpp
- **Acceptance**: 2 test files pass. Phase 8 tests unchanged.
- **Size**: S

### Gate M9.3
- [ ] All Phase 8 + 9.1 + 9.2 tests pass
- [ ] 2 new compliance tests pass
- [ ] vector-consistency CI job green
- [ ] CI green on 4 platforms

---

## Sub-phase 9.4 — LaTeX Math via MicroTeX

### T12: MicroTeX vendor + MathRenderer
- **Owner**: Backend
- **Files**: cmake/FetchMicroTeX.cmake (NEW),
  src/lumen/export/MathRenderer.{h,cpp}
- **Acceptance**: MicroTeX builds statically on Ubuntu+macOS.
  MathRenderer::render("\\sigma^2", 12, Qt::black) returns
  non-empty QImage. renderToPath() returns non-empty QPainterPath.
  isValidLatex() returns false for malformed input, no crash.
- **Size**: L

### T13: LaTeX mode toggles in 3 dialogs
- **Owner**: Frontend
- **Files**: src/lumen/ui/AxisDialog.{h,cpp},
  src/lumen/ui/TitleDialog.{h,cpp},
  src/lumen/ui/LegendDialog.{h,cpp} (all extended)
- **Acceptance**: Each dialog gains a "LaTeX" checkbox. When
  checked, label text is rendered via MathRenderer in the
  preview and in PlotRenderer output.
- **Size**: M

### T14: PlotRenderer LaTeX integration
- **Owner**: Backend
- **Files**: src/lumen/plot/PlotRenderer.{h,cpp} (extended)
- **Acceptance**: When a label/title has LaTeX mode enabled,
  PlotRenderer calls MathRenderer::renderToPath() instead of
  QPainter::drawText(). Vector paths appear in SVG/PDF export.
- **Size**: M

### T15: MicroTeX tests
- **Owner**: QA
- **Files**: tests/unit/test_microtex_basic.cpp,
  test_microtex_complex.cpp, test_microtex_invalid.cpp,
  test_axis_label_latex.cpp, test_legend_latex.cpp
- **Acceptance**: 5 test files pass. Phase 8 tests unchanged.
- **Size**: M

### Gate M9.4
- [ ] All previous tests pass
- [ ] 5 new LaTeX tests pass
- [ ] CI green on 4 platforms + vector-consistency
- [ ] LaTeX in axis labels, title, legend renders in PNG/SVG/PDF

---

## Sub-phase 9.5 — Annotation Layer

### T16: AnnotationLayer base + Annotation abstract
- **Owner**: Backend
- **Files**: src/lumen/plot/AnnotationLayer.{h,cpp},
  src/lumen/plot/Annotation.{h,cpp}
- **Acceptance**: AnnotationLayer::addAnnotation(),
  removeAnnotation(), paint(), hitTest() compile and work with
  a trivial concrete Annotation stub.
- **Size**: M

### T17: 6 concrete Annotation classes
- **Owner**: Backend
- **Files**: src/lumen/plot/annotations/ArrowAnnotation.{h,cpp},
  BoxAnnotation.{h,cpp}, CalloutAnnotation.{h,cpp},
  TextAnnotation.{h,cpp}, ScaleBar.{h,cpp}, ColorBar.{h,cpp}
- **Acceptance**: Each class: paint(), boundingRect(), toJson(),
  fromJson() static factory. TextAnnotation supports inline
  LaTeX via $...$ (delegates to MathRenderer). ScaleBar computes
  length from data units. ColorBar maps to Heatmap colormap.
- **Size**: XL

### T18: ChangeAnnotation* commands (bundled pattern)
- **Owner**: Backend
- **Files**: src/lumen/core/commands/ChangeAnnotationCommand.{h,cpp}
  (one polymorphic command class, dispatches per type)
- **Acceptance**: execute/undo/description work. Round-trip through
  CommandBus.
- **Size**: M

### T19: AnnotationToolbar
- **Owner**: Frontend
- **Files**: src/lumen/ui/AnnotationToolbar.{h,cpp} (NEW)
- **Acceptance**: Floating toolbar on PlotCanvas with 6 buttons.
  Click button → enter placement mode → click on plot → annotation
  created at click position. Works with InteractionController.
- **Size**: L

### T20: AnnotationPropertyDialog (polymorphic)
- **Owner**: Frontend
- **Files**: src/lumen/ui/AnnotationPropertyDialog.{h,cpp} (NEW)
- **Acceptance**: Double-click annotation → dialog opens with
  type-specific controls. OK → ChangeAnnotationCommand via
  CommandBus. Cancel → no change.
- **Size**: L

### T21: WorkspaceFile annotations serialization
- **Owner**: Backend
- **Files**: src/lumen/core/io/WorkspaceFile.{h,cpp} (extended)
- **Acceptance**: captureFromScene includes "annotations" array.
  applyToScene restores all 6 annotation types. Round-trip test.
- **Size**: M

### T22: Annotation tests (10)
- **Owner**: QA
- **Files**: tests/unit/test_annotation_layer.cpp,
  test_arrow_annotation.cpp, test_box_annotation.cpp,
  test_callout_annotation.cpp, test_text_annotation.cpp,
  test_scale_bar.cpp, test_color_bar.cpp,
  test_annotation_toolbar.cpp, test_annotation_dialog.cpp,
  test_annotation_workspace.cpp
- **Acceptance**: 10 test files pass. Phase 8 tests unchanged.
- **Size**: L

### Gate M9.5
- [ ] All previous tests pass
- [ ] 10 new annotation tests pass
- [ ] CI green on 4 platforms + vector-consistency
- [ ] All 6 annotation types paint correctly
- [ ] Toolbar placement mode works
- [ ] Property dialogs via CommandBus
- [ ] Workspace roundtrip preserves annotations
- [ ] Human creates publication figure with annotations + PDF export

---

## Sub-phase 9.6 — Async Export Pipeline

### T23: ExportTask with QThread + cooperative cancel
- **Owner**: Backend
- **Files**: src/lumen/export/ExportTask.{h,cpp}
- **Acceptance**: ExportTask::start() renders in background thread.
  progress(int) signal emitted. cancel() sets atomic flag, render
  loop checks between primitives, no output file produced. Atomic
  file write (temp + rename).
- **Size**: M

### T24: ExportProgressDialog + ExportDialog async wiring
- **Owner**: Frontend
- **Files**: src/lumen/ui/ExportProgressDialog.{h,cpp} (NEW),
  src/lumen/ui/ExportDialog.{h,cpp} (extended)
- **Acceptance**: ExportDialog "Export" button starts ExportTask.
  ExportProgressDialog shows progress bar + step label + Cancel
  button. Cancel stops export. Finished signal closes dialog with
  success/error message.
- **Size**: M

### T25: Async export tests
- **Owner**: QA
- **Files**: tests/unit/test_export_task_progress.cpp,
  test_export_task_cancel.cpp, test_export_progress_dialog.cpp
- **Acceptance**: 3 test files pass. Phase 8 tests unchanged.
- **Size**: M

### Gate M9.6
- [ ] All previous tests pass
- [ ] 3 new async tests pass
- [ ] CI green on 4 platforms + vector-consistency
- [ ] Large export shows progress
- [ ] Cancel produces no output, no crash

---

## Closing — T26

### T26: Docs + review + close
- **Owner**: Docs + Architect + Integration
- **Files**: README.md (updated), src/lumen/export/CLAUDE.md (NEW),
  src/lumen/plot/CLAUDE.md (updated for AnnotationLayer),
  docs/reviews/phase-9-review.md (NEW), .lumen-ops/STATUS.md
- **Acceptance**: phase-9-review.md WRITTEN AND COMMITTED IN THE
  SAME COMMIT AS THE CLOSING .lumen-ops/STATUS.md ENTRY. Review
  includes ONE-LINE VERIFICATION NOTE FOR EACH user-visible exit
  checklist item (e.g., "verified: ICC PDF opens in Preview with
  correct colors"). This rule appears verbatim here because Phase
  8's review lacked per-deliverable verification and the premature
  close incident resulted. vphase-9 tag pushed.
- **Size**: M

---

## Test budget

| Sub-phase | New tests | Cumulative |
|-----------|----------|------------|
| Phase 8 baseline | — | 700 |
| 9.1 ICC | 5 | 705 |
| 9.2 Fonts | 4 | 709 |
| 9.3 Vector | 2 | 711 |
| 9.4 LaTeX | 5 | 716 |
| 9.5 Annotations | 10 | 726 |
| 9.6 Async | 3 | 729 |
| **Total** | **29** | **729** |

Target 729 minimum; stretch to 850+ with edge case and
integration tests.
