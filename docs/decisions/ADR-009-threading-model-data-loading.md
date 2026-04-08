# ADR-009: Threading model for data loading

## Status
Accepted (Phase 1)

## Context
CSV files can be large (100 MB+). Parsing on the main thread freezes
the UI. We need a threading strategy that keeps the event loop
responsive while loading data, and safely delivers the result to the
main thread for display.

Qt offers several concurrency primitives: `QThread`,
`QThreadPool` + `QRunnable`, `QtConcurrent::run`, and raw
`std::thread` + `std::future`.

## Decision
Use `QThread` with a `QObject`-based worker (`FileLoader`) for file
loading:

1. `FileLoader` is a `QObject` moved to a `QThread`.
2. The worker runs `CsvReader` synchronously inside the thread.
3. Progress is reported via a `progress(int percent)` signal
   (queued connection to main thread).
4. On completion, a `finished(DataFrame)` signal delivers the
   result. `DataFrame` is moved, not copied.
5. On error, a `failed(CsvError)` signal carries the error.
6. Cancellation via an `std::atomic<bool>` flag checked between
   row batches (~every 1000 rows).

The thread is created per load operation and destroyed on
completion (not pooled). For Phase 1 we load one file at a time.

## Consequences
- + UI stays responsive during loading
- + Progress bar possible (signal-based)
- + Clean cancellation path
- + Familiar Qt pattern; easy to debug with sanitizers
- - One thread per load (fine for Phase 1; revisit if we need
  concurrent multi-file loads)
- - `DataFrame` must be move-safe across threads (it is: move-only
  value type)

## Alternatives considered
- **`QtConcurrent::run`**: simpler API but harder to report
  incremental progress and cancel mid-parse. Better for short
  fire-and-forget tasks.
- **`QThreadPool` + `QRunnable`**: good for many small tasks, but
  file loading is one large task. Overkill for Phase 1.
- **`std::thread` + `std::future`**: loses Qt's queued signal
  delivery; requires manual event posting. More error-prone.
