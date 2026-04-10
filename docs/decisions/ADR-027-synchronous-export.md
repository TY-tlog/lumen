# ADR-027: Synchronous export in main thread

## Status
Accepted (Phase 4)

## Context
FigureExporter renders a PlotScene to a file (PNG, SVG, or PDF).
The rendering involves QPainter operations on a QImage,
QSvgGenerator, or QPdfWriter. The question is whether this should
run synchronously in the main thread or asynchronously in a worker
thread with a progress bar.

## Decision
Phase 4 exports run synchronously in the main thread. The
ExportDialog is non-modal, but the actual export call
(FigureExporter::exportFigure) blocks the main thread briefly
while rendering and writing the file.

Rationale: current plot sizes are small. The reference
electrophysiology data has 3,500 points per series, 1-2 series.
At 300 DPI, a "Publication single column" (1050x700 px) PNG
renders in < 50 ms. SVG and PDF are similarly fast because the
PlotRenderer draws the same number of primitives regardless of
target.

The user perceives no delay. A progress bar would add complexity
(thread management, cancellation, QPainter thread safety) with
zero user benefit at current data sizes.

## Consequences
- + Simple: one function call, no threading
- + No QPainter thread-safety concerns (QPainter is not thread-
  safe by default; QImage can be painted in a worker thread but
  QSvgGenerator and QPdfWriter cannot)
- + ExportDialog can show a simple "Exporting..." cursor change
  rather than a progress bar
- - If export takes > 500 ms, the UI freezes visibly. This is the
  trigger for revisiting this decision.
- - No cancellation support. Acceptable because the operation is
  fast enough that the user won't want to cancel.

## Trigger for revisit
If any export on the project owner's data takes > 500 ms, move
to a background-thread model with progress bar. This would require:
1. Render to QImage in a worker thread (QImage is thread-safe)
2. Save QImage to file in the worker thread
3. For SVG/PDF, investigate thread safety or render to QImage first
   then convert
4. Deliver result via queued signal to main thread

Estimated work to add background export: 4-6 hours (Phase 5+).

## Alternatives considered
- **Background thread with progress bar**: the "correct"
  architecture for potentially slow operations. Rejected for
  Phase 4 because the operation is fast (<50ms) and the threading
  complexity is not justified. QSvgGenerator is not documented as
  thread-safe, which would require rendering SVG in the main
  thread anyway or rendering to QImage first.
- **QConcurrent::run**: simpler than manual QThread but still
  adds threading. Rejected for the same reason: no user benefit
  at current data sizes.
