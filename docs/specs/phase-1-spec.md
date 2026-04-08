# Phase 1 — Data Layer and First UI Shell

## Goal

Load a CSV file, parse it into an in-memory column store, and
display the data in a table view inside the Lumen window. Apply the
first pass of the Apple-mood design system (QSS, Inter font). No
plotting yet — that is Phase 2.

## Active agents

- Architect (plan, ADRs, design updates)
- Backend (CSV parser, data model, core services)
- Frontend (UI shell, DataTableDock, style system)
- QA (fixtures, unit tests, integration tests)
- Integration (merge PRs, keep main green)
- Docs (update README, CLAUDE.md sync)

## Reference data

Primary test file: electrophysiology patch-clamp recording.
- 9 columns: `time_ms`, `voltage_mV`, `current_stimulus_nA`,
  `I_ion_nA`, `dvdt_2pt_fwd`, `dvdt_3pt_fwd`, `dvdt_3pt_bwd`,
  `dvdt_2pt_cen`, `dvdt_5pt_cen`
- 3499 data rows, 472 KB
- All columns are `double` (with NaN at boundaries)
- Comma-delimited, LF line endings, no BOM, no quoting
- Header row present

## Deliverables

### D1 — CSV Parser (`src/lumen/data/`)
- [x] `CsvReader` class: streaming row-callback parser
- [x] RFC 4180 compliance: quoted fields, escaped quotes, CRLF/LF/CR
- [x] Configurable delimiter (default `,`)
- [x] UTF-8 input (BOM-aware); reject non-UTF-8 with clear error
- [x] NaN handling: `NaN`, `nan`, `NAN`, `NA`, empty cell → null/NaN
- [x] Type inference per column: scan first N rows (default 100),
      classify as int → double → string. DateTime deferred to Phase 3.
- [x] Header detection: if first row is all non-numeric strings,
      treat as header
- [x] Error reporting: line number + column number + description
- [x] Performance target: 100 MB CSV < 5 seconds on dev machine

### D2 — Column-Store Data Model (`src/lumen/data/`)
- [x] `DataFrame` class: owns a vector of `Column` objects
- [x] `Column`: type-erased column (int64, double, string)
- [x] Double columns store NaN for null values
- [x] `ColumnType` enum: `Int64`, `Double`, `String`
- [x] Row count, column count, column name lookup
- [x] Value access by (row, col) with bounds checking
- [x] Move-only semantics (no copy)

### D3 — Core Services (`src/lumen/core/`)
- [x] `EventBus`: lightweight publish/subscribe for cross-module
      decoupling. Uses Qt signals under the hood but provides a
      typed, enum-based event interface.
- [x] `DocumentRegistry`: tracks open files. Owns `DataFrame`
      instances. Emits events on open/close.
- [x] File-loading uses `QThread` worker: main thread stays
      responsive, progress reported via signal, result delivered
      via queued connection.

### D4 — File Open Flow (`src/lumen/ui/` + `src/lumen/app/`)
- [x] File → Open CSV... (Ctrl+O): `QFileDialog` with CSV filter
- [x] Progress indication during load (status bar message or
      simple progress dialog for files > 1 MB)
- [x] On success: `DocumentRegistry` stores the `DataFrame`,
      `DataTableDock` shows the data
- [x] On error: `QMessageBox` with parser error details
- [x] Recent files in File menu (QSettings-backed, last 10)

### D5 — DataTableDock (`src/lumen/ui/`)
- [x] `QDockWidget` with a `QTableView` and custom
      `DataFrameTableModel` (read-only `QAbstractTableModel`)
- [x] Column headers from CSV header row
- [x] NaN cells displayed as "NaN" in text.tertiary color
- [x] Double values displayed with up to 6 significant digits
- [x] Column sorting by click on header
- [x] Dock is toggled from View menu

### D6 — Apple-Mood QSS v1 (`src/lumen/style/`)
- [x] `DesignTokens` class: C++ constants from design-system.md
- [x] `StyleManager`: loads Inter font, generates and applies QSS
- [x] Light theme only (dark theme deferred to Phase 3)
- [x] Styled: window background, menu bar, status bar, dock title
      bars, table view, scroll bars, message boxes
- [x] Inter font loaded from `resources/fonts/Inter-*.ttf`

### D7 — Test Fixtures (`tests/fixtures/tiny/`)
- [x] `simple_3x4.csv` — 3 columns, 4 rows, pure doubles
- [x] `with_header.csv` — header + mixed int/double/string
- [x] `with_nan.csv` — contains NaN, empty cells
- [x] `electrophys_sample.csv` — first 20 rows of reference data
- [x] `rfc4180_quoted.csv` — quoted fields, commas in values
- [x] `no_header.csv` — pure numeric, no header row
- [x] `empty.csv` — empty file (0 bytes)
- [x] `single_column.csv` — edge case
- [x] `large_100k.csv` — generated, 100K rows for perf test

## Non-goals (deferred)

- Plot engine (Phase 2)
- Data editing / mutation (Phase 4)
- Dark theme (Phase 3)
- DateTime type inference (Phase 3)
- Export / save-as (Phase 4)
- CP949 / non-UTF-8 encoding support (Phase 3)
- Property inspector (Phase 5)
- Multiple file tabs (Phase 3)

## Acceptance criteria

```bash
# Build and test
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DLUMEN_ENABLE_ASAN=ON -DLUMEN_ENABLE_UBSAN=ON
cmake --build build
cd build && ctest --output-on-failure

# Manual verification
./build/bin/lumen
# 1. File → Open CSV → select a CSV file
# 2. Data appears in DataTableDock table view
# 3. NaN values shown in grey
# 4. Column headers match CSV headers
# 5. Click column header to sort
# 6. Inter font visible in UI elements
# 7. Apple-mood styling: light background, rounded elements
# 8. File → recent files shows last opened file
# 9. Ctrl+Q quits cleanly, no ASan/UBSan errors
```

## Risks

- **NaN handling inconsistency** between parser and table model.
  Mitigated by shared `ColumnType` enum and explicit NaN convention.
- **Large file blocking UI**. Mitigated by worker-thread loading
  with progress signal.
- **Inter font licensing**. Inter is OFL-licensed (free). Bundle
  `.ttf` files in `resources/fonts/`. See ADR-011.
- **Qt 6.4 vs 6.7 API gaps**. Our Ubuntu apt install gives 6.4.2.
  Avoid APIs added after 6.4. `qt_add_executable` and
  `qt_standard_project_setup` are available since 6.2.

## Exit checklist

- [ ] All deliverables done
- [ ] CI green on main
- [ ] Owner can open their electrophysiology CSV and see data
- [ ] `docs/reviews/phase-1-review.md` written
- [ ] `docs/specs/phase-2-spec.md` drafted
