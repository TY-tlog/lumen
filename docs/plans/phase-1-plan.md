# Phase 1 Plan — Data Layer and First UI Shell

> Reference: `docs/specs/phase-1-spec.md`

## Task Dependency Graph

```
                    ┌─────────────┐
                    │ T1 CsvReader │ ← critical path start
                    │  (backend)   │
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
     ┌────────────┐ ┌───────────┐ ┌──────────────┐
     │T2 DataFrame│ │T7 Fixtures│ │T8 Style/QSS  │
     │ (backend)  │ │   (qa)    │ │ (frontend)   │
     └─────┬──────┘ └─────┬─────┘ └──────┬───────┘
           │              │              │
           ▼              │              ▼
  ┌─────────────────┐     │     ┌────────────────┐
  │T3 EventBus +    │     │     │T9 Inter font   │
  │ DocumentRegistry│     │     │  (frontend)    │
  │   (backend)     │     │     └────────┬───────┘
  └────────┬────────┘     │              │
           │              │              │
           ▼              │              ▼
  ┌─────────────────┐     │     ┌────────────────┐
  │T4 Worker thread │     │     │T10 DataTable   │
  │ file loader     │     │     │  Dock shell    │
  │   (backend)     │     │     │  (frontend)    │
  └────────┬────────┘     │     └────────┬───────┘
           │              │              │
           └──────────┬───┘──────────────┘
                      ▼
             ┌─────────────────┐
             │T5 File-open     │
             │ integration     │
             │ (frontend+back) │
             └────────┬────────┘
                      ▼
             ┌─────────────────┐
             │T6 Integration   │
             │ test: open CSV  │
             │    (qa)         │
             └────────┬────────┘
                      ▼
             ┌─────────────────┐
             │T11 Docs update  │
             │   (docs)        │
             └─────────────────┘
```

## Critical path

T1 → T2 → T3 → T4 → T5 → T6

Estimated: ~8–10 working days on the critical path.
T7, T8, T9, T10 run in parallel off the critical path.

---

## Tasks

### T1 — CsvReader (Backend, Size: L)

**Owner**: backend

**Files to create/modify**:
- `src/lumen/data/CsvReader.h`
- `src/lumen/data/CsvReader.cpp`
- `src/lumen/data/CsvError.h` (error types)
- `src/lumen/data/CMakeLists.txt` (new; data module library)
- `src/lumen/CMakeLists.txt` (link data module)
- `tests/unit/test_csv_reader.cpp`

**Subtasks**:
- T1.1: Tokenizer — split line into fields respecting RFC 4180
  quoting. Handle LF, CRLF, CR.
- T1.2: NaN detection — recognize `NaN`, `nan`, `NAN`, `NA`,
  empty string as null/NaN.
- T1.3: Type inference — scan first 100 rows, classify each column
  as int64 / double / string. All-NaN column → double.
- T1.4: Header detection — if row 0 is all non-numeric, treat as
  header; otherwise generate `col_0`, `col_1`, etc.
- T1.5: Streaming parse — row callback or direct DataFrame build.
  UTF-8 BOM detection and skip.
- T1.6: Error reporting — `CsvError` with line, column, message.
- T1.7: Unit tests for all of the above.

**Acceptance criteria**:
- Parses reference CSV (3500 rows, 9 cols) correctly
- NaN values preserved as `std::numeric_limits<double>::quiet_NaN()`
- Quoted fields with embedded commas and newlines work
- Empty file returns empty DataFrame without error
- Malformed rows produce clear `CsvError`
- All unit tests pass under ASan + UBSan

**Dependencies**: none (can start immediately)

---

### T2 — DataFrame and Column (Backend, Size: M)

**Owner**: backend

**Files to create/modify**:
- `src/lumen/data/DataFrame.h`
- `src/lumen/data/DataFrame.cpp`
- `src/lumen/data/Column.h`
- `src/lumen/data/Column.cpp`
- `src/lumen/data/ColumnType.h` (enum)
- `tests/unit/test_dataframe.cpp`

**Subtasks**:
- T2.1: `ColumnType` enum (`Int64`, `Double`, `String`)
- T2.2: `Column` class — type-erased, stores
  `std::vector<int64_t>` / `std::vector<double>` /
  `std::vector<QString>`. Name, type, row count.
- T2.3: `DataFrame` — owns `std::vector<Column>`, provides
  column-by-name lookup, row/col counts, value access.
- T2.4: Move-only semantics, no copy.
- T2.5: Unit tests.

**Acceptance criteria**:
- Can construct a DataFrame from CsvReader output
- Column lookup by name returns correct column
- Bounds-checked access throws or returns error on out-of-range
- Move construction works, copy is deleted
- ASan/UBSan clean

**Dependencies**: T1 (CsvReader produces the data that fills DataFrame)

---

### T3 — EventBus and DocumentRegistry (Backend, Size: M)

**Owner**: backend

**Files to create/modify**:
- `src/lumen/core/EventBus.h`
- `src/lumen/core/EventBus.cpp`
- `src/lumen/core/DocumentRegistry.h`
- `src/lumen/core/DocumentRegistry.cpp`
- `src/lumen/core/CMakeLists.txt` (new; core module library)
- `src/lumen/CMakeLists.txt` (link core module)
- `tests/unit/test_event_bus.cpp`
- `tests/unit/test_document_registry.cpp`

**Subtasks**:
- T3.1: `EventBus` — QObject-based, typed event enum, subscribe
  with callback or slot, emit fires all subscribers. Thread-safe
  emit via queued connections.
- T3.2: `DocumentRegistry` — owns open `DataFrame` instances keyed
  by file path. Signals: `documentOpened(QString path)`,
  `documentClosed(QString path)`.
- T3.3: Unit tests.

**Acceptance criteria**:
- EventBus delivers events to all subscribers
- DocumentRegistry stores and retrieves DataFrames
- Opening same file twice returns existing document (no duplicate)
- Closing removes document and emits signal
- Thread-safe: emit from worker thread delivers on main thread

**Dependencies**: T2 (DocumentRegistry stores DataFrames)

---

### T4 — Worker-Thread File Loader (Backend, Size: M)

**Owner**: backend

**Files to create/modify**:
- `src/lumen/data/FileLoader.h`
- `src/lumen/data/FileLoader.cpp`
- `tests/unit/test_file_loader.cpp`

**Subtasks**:
- T4.1: `FileLoader` — QObject, runs CsvReader in a QThread.
  Signals: `progress(int percent)`, `finished(DataFrame result)`,
  `failed(CsvError error)`.
- T4.2: Cancellation support (atomic flag checked between rows).
- T4.3: Integration with DocumentRegistry: on success, register
  the document.
- T4.4: Unit test with small fixture file.

**Acceptance criteria**:
- Main thread not blocked during parsing
- Progress signal emitted at least every 10%
- Cancellation stops parsing within 100 ms
- Result delivered via queued signal to main thread
- ASan/UBSan clean

**Dependencies**: T1, T2, T3

---

### T5 — File-Open UI Flow (Frontend + Backend, Size: M)

**Owner**: frontend (UI), with backend's T4 already done

**Files to create/modify**:
- `src/lumen/ui/MainWindow.cpp` (add Open action, recent files)
- `src/lumen/ui/MainWindow.h`
- `src/lumen/app/Application.cpp` (create core services)
- `src/lumen/app/Application.h`

**Subtasks**:
- T5.1: File → Open CSV (Ctrl+O) opens QFileDialog with
  `*.csv` filter. Calls FileLoader.
- T5.2: Progress feedback: status bar message "Loading file..."
  or QProgressDialog for files > 1 MB.
- T5.3: On success: DocumentRegistry stores it, DataTableDock
  shows data.
- T5.4: On error: QMessageBox with CsvError details.
- T5.5: Recent files: QSettings list, last 10, shown in File menu.

**Acceptance criteria**:
- User can open a CSV file through the GUI
- Large file does not freeze the window
- Error on bad file is shown in a message box
- Recent files list updates and works on re-launch
- Ctrl+O shortcut works

**Dependencies**: T4 (FileLoader), T10 (DataTableDock shell)

---

### T6 — Integration Test: Open CSV End-to-End (QA, Size: S)

**Owner**: qa

**Files to create/modify**:
- `tests/integration/test_open_csv.cpp`
- `tests/integration/CMakeLists.txt` (new)
- `tests/CMakeLists.txt` (add integration subdir)

**Subtasks**:
- T6.1: Test that opening `electrophys_sample.csv` through
  DocumentRegistry produces correct DataFrame dimensions and values.
- T6.2: Test NaN handling end-to-end.
- T6.3: Test error path with malformed CSV.

**Acceptance criteria**:
- Integration test compiles and passes
- Exercises the full path: file → CsvReader → DataFrame →
  DocumentRegistry
- ASan/UBSan clean

**Dependencies**: T4, T7 (fixtures)

---

### T7 — Test Fixtures (QA, Size: S) ⚡ parallel

**Owner**: qa

**Files to create/modify**:
- `tests/fixtures/tiny/simple_3x4.csv`
- `tests/fixtures/tiny/with_header.csv`
- `tests/fixtures/tiny/with_nan.csv`
- `tests/fixtures/tiny/electrophys_sample.csv`
- `tests/fixtures/tiny/rfc4180_quoted.csv`
- `tests/fixtures/tiny/no_header.csv`
- `tests/fixtures/tiny/empty.csv`
- `tests/fixtures/tiny/single_column.csv`
- `scripts/generate_large_fixture.py` (generates large_100k.csv)

**Subtasks**:
- T7.1: Hand-craft 8 small CSV fixtures.
- T7.2: Write Python script to generate 100K-row perf fixture.
- T7.3: Add `electrophys_sample.csv` (first 20 rows of reference
  data, anonymized column values are fine since it's just numbers).

**Acceptance criteria**:
- All fixtures are valid (or intentionally invalid) per their name
- `electrophys_sample.csv` has 9 columns, 20 rows, includes NaN
- Fixtures are committed, not gitignored

**Dependencies**: none (can start immediately, parallel with T1)

---

### T8 — Apple-Mood QSS v1 and DesignTokens (Frontend, Size: M) ⚡ parallel

**Owner**: frontend

**Files to create/modify**:
- `src/lumen/style/DesignTokens.h`
- `src/lumen/style/DesignTokens.cpp`
- `src/lumen/style/StyleManager.h`
- `src/lumen/style/StyleManager.cpp`
- `src/lumen/style/CMakeLists.txt` (new; style module library)
- `src/lumen/CMakeLists.txt` (link style module)
- `resources/styles/light.qss` (generated or static)
- `tests/unit/test_design_tokens.cpp`

**Subtasks**:
- T8.1: `DesignTokens` — C++ constexpr/const values mirroring
  `docs/design/design-system.md` light palette. Colors as QColor,
  sizes as int, radii as int.
- T8.2: `StyleManager` — singleton-like, loads fonts, applies QSS.
  Called from `Application::loadStyleSheet()`.
- T8.3: QSS file for light theme: window background, menu bar,
  dock title bars, table view, scroll bars.
- T8.4: Unit test that DesignTokens values match design-system.md.

**Acceptance criteria**:
- App visually matches Apple-mood design system (light theme)
- No literal color/size values in widget code
- QSS applies without runtime errors
- DesignTokens test passes

**Dependencies**: none (can start immediately, parallel with T1)

---

### T9 — Inter Font Integration (Frontend, Size: S) ⚡ parallel

**Owner**: frontend

**Files to create/modify**:
- `resources/fonts/Inter-Regular.ttf`
- `resources/fonts/Inter-Medium.ttf`
- `resources/fonts/Inter-SemiBold.ttf`
- `resources/fonts/Inter-Bold.ttf`
- `resources/fonts/OFL.txt` (license)
- `src/lumen/style/StyleManager.cpp` (font loading)
- `src/lumen/CMakeLists.txt` (qt_add_resources for fonts)

**Subtasks**:
- T9.1: Download Inter font files from https://rsms.me/inter/
- T9.2: Add OFL license to `resources/fonts/`
- T9.3: Register fonts via `QFontDatabase::addApplicationFont()`
  in StyleManager
- T9.4: Set application default font to Inter

**Acceptance criteria**:
- Inter font renders in all UI text on Linux
- OFL license file present
- Falls back gracefully if font files missing

**Dependencies**: T8 (StyleManager exists)

---

### T10 — DataTableDock Shell (Frontend, Size: M) ⚡ parallel

**Owner**: frontend

**Files to create/modify**:
- `src/lumen/ui/DataTableDock.h`
- `src/lumen/ui/DataTableDock.cpp`
- `src/lumen/ui/DataFrameTableModel.h`
- `src/lumen/ui/DataFrameTableModel.cpp`
- `src/lumen/ui/MainWindow.cpp` (add dock)
- `tests/unit/test_dataframe_table_model.cpp`

**Subtasks**:
- T10.1: `DataFrameTableModel` — read-only QAbstractTableModel
  backed by a DataFrame pointer. Displays headers, formats doubles
  to 6 significant digits, shows NaN in text.tertiary color.
- T10.2: `DataTableDock` — QDockWidget wrapping QTableView.
  Column sort by header click. Toggle from View menu.
- T10.3: Unit test for DataFrameTableModel (row/col counts, data
  formatting, NaN display role).

**Acceptance criteria**:
- Dock shows column headers from CSV
- Double values formatted to 6 significant digits
- NaN shown as "NaN" in grey (text.tertiary)
- Clicking column header sorts data
- View → Data Table toggles the dock
- Dock remembers visibility in QSettings

**Dependencies**: T2 (DataFrame), T8 (style tokens for NaN color)

---

### T11 — Docs Update (Docs, Size: S)

**Owner**: docs

**Files to create/modify**:
- `README.md` (update Build, usage instructions)
- `src/lumen/data/CLAUDE.md` (update with actual responsibilities)
- `src/lumen/core/CLAUDE.md` (update)
- `src/lumen/style/CLAUDE.md` (update)
- `src/lumen/ui/CLAUDE.md` (update)
- `CLAUDE.md` (update current phase to Phase 1)

**Acceptance criteria**:
- README reflects that Lumen can open CSV files
- Per-module CLAUDE.md matches actual code

**Dependencies**: T5, T6 (after features are merged)

---

## Parallel Execution Schedule

```
Week 1:
  Backend:  T1 (CsvReader) ──────────────────────►
  Frontend: T8 (QSS/tokens) ─────► T9 (font) ──►
  QA:       T7 (fixtures) ────►

Week 2:
  Backend:  T2 (DataFrame) ──► T3 (EventBus/Registry) ──►
  Frontend: T10 (DataTableDock shell) ──────────────────►
  QA:       (review Backend PRs, add tests)

Week 3:
  Backend:  T4 (FileLoader worker thread) ──►
  Frontend: T5 (file-open UI integration) ──────────────►
  QA:       T6 (integration test) ──►

Week 3-4:
  Integration: merge PRs
  Docs:        T11
```

## Agent Launch Order

1. **Now**: Architect only (this session). Plan → human approval → merge to main.
2. **After plan merged**: Backend + Frontend + QA simultaneously.
3. **When PRs start accumulating**: Integration.
4. **After first PRs merged**: Docs.
