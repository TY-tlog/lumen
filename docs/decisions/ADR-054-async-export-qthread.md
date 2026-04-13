# ADR-054: Async export with QThread + cooperative cancel

## Status
Proposed (Phase 9)

## Context
Exporting a 4K PNG at 600 DPI or a complex PDF with embedded
fonts can take several seconds. The UI must remain responsive
with a progress indicator and cancel option.

## Decision
ExportTask runs the full rendering pipeline in a QThread.

### Thread model
```
Main thread                  Worker thread
───────────                  ─────────────
ExportTask::start()
  → moveToThread(worker)
  → worker.start()
                             render to QImage/temp file
                             emit progress(percent)
                             check m_cancelRequested
                             ...
                             write temp file
                             atomic rename to final path
                             emit finished(success, path)
```

### Thread safety guarantees
1. PlotScene is READ-ONLY during export. Worker thread calls
   PlotRenderer::render() which only reads PlotScene state.
   No mutations occur during export.
2. If the user edits the plot during export, the export captures
   a snapshot of the current state at start(). (Implementation:
   ExportTask copies the PlotScene state it needs, or takes a
   shared_ptr to an immutable snapshot.)
3. Progress signals are queued connections (Qt::QueuedConnection).
4. m_cancelRequested is std::atomic<bool>.

### Cooperative cancel
PlotRenderer::render() is extended with an optional
`std::atomic<bool>* cancelFlag` parameter. Between rendering
each PlotItem (and each annotation), it checks the flag. If
set, render returns early. ExportTask detects incomplete render
and does NOT write the output file.

### Atomic file write
Export always writes to a temporary file (same directory,
".lumen_tmp" suffix). On successful completion, QFile::rename()
atomically replaces the target. On cancel or error, the temp
file is deleted. This prevents corrupt partial output.

## Consequences
- + UI remains responsive during large exports
- + Cancel is instant (cooperative check between items)
- + No corrupt output files (atomic rename)
- + Progress feedback improves UX
- - PlotRenderer gains an optional cancelFlag parameter
  (minimal API change)
- - Snapshot of PlotScene adds brief allocation overhead at
  export start
- - QThread lifecycle management (start, wait, delete)

## Alternatives considered
- **Process-based** (spawn separate process for export):
  Rejected. Overhead of process creation, IPC for PlotScene
  state, and platform-specific process management. Overkill.
- **Preemptive cancel** (kill the thread): Rejected. Qt
  rendering is not preemptible. Killing a thread during QPainter
  operations corrupts shared state. Cooperative cancel is the
  only safe approach.
- **Keep synchronous, add "please wait" dialog**: Rejected.
  Blocks the entire UI. User cannot cancel. Bad UX for large
  exports (>5 seconds).
