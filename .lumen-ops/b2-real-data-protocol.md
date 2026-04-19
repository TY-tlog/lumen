# B2: Real-Data Verification Protocol

**Owner**: T.Y. Kim
**Date**: 2026-04-18
**Status**: Ready for execution

---

## Why this matters

Every phase review since Phase 1 verified "unit tests pass" but never
verified "the owner can use this tool for their actual work." Phase 8
proved that passing 666 unit tests can coexist with a non-functional
render pipeline. This protocol verifies the **purpose** of the app,
not just the **parts**.

---

## Owner's data inventory (found on system)

| File | Format | Size | Notes |
|------|--------|------|-------|
| `~/Desktop/yoondata/Ion_data/simulation_results/results_Ca_out_*.csv` | CSV (columnar) | 8 files, ~80 cols each | AP metrics at different [Ca]_out. Standard header+rows format. **Should load.** |
| `~/Desktop/yoondata/Ion_data/voltage_timeseries.csv` | CSV (transposed) | 502 rows × thousands of cols | Variables as rows, time as columns. **Non-standard — may fail.** |
| `~/Desktop/yoondata/RPS/Trace/RPS_intra_*.h5` | HDF5 | 11 files | Intracellular voltage traces. Depends on internal HDF5 structure. |

---

## Step 1 — Load real data

Open Lumen and try loading each file type:

### 1a. Columnar CSV (expected to work)
```
File > Open > ~/Desktop/yoondata/Ion_data/simulation_results/results_Ca_out_0.63mM.csv
```

**Check**:
- [ ] File loads without error
- [ ] Data Table shows all ~80 columns with correct headers
  (Ca_out, AP_count, time_step, avg_membrane_potential, AP1_min_V, ...)
- [ ] NaN values display correctly (grey text)
- [ ] Column sorting works
- [ ] Window title shows filename

**If it fails**: screenshot the error, note the error message.

### 1b. Transposed CSV (may fail — this is a finding)
```
File > Open > ~/Desktop/yoondata/Ion_data/voltage_timeseries.csv
```

This file has variable names in the first column and time points as
column headers. Lumen's CSV parser expects column headers in the
first row. Expected behavior: loads but with wrong structure (treats
time points as column names, gives ~500 rows of numbers).

**Check**:
- [ ] Does it load at all?
- [ ] Is the data structured correctly?
- [ ] If not: note what you expected vs. what you got

### 1c. HDF5 traces
```
File > Open > ~/Desktop/yoondata/RPS/Trace/RPS_intra_0.1.h5
```

**Check**:
- [ ] File loads (HDF5 loader recognizes the internal structure)
- [ ] If it fails: note the error message

---

## Step 2 — Reproduce a real figure

Using the columnar CSV data (`results_Ca_out_0.63mM.csv`), try to
create a plot that you would actually use in your research:

### Suggested figures (pick one or more):

**Figure A: AP peak voltage vs. calcium concentration**
- X axis: Ca_out (from multiple result files, or from one file's column)
- Y axis: AP1_max_V
- Plot type: Scatter or Line
- Goal: show how extracellular calcium affects action potential amplitude

**Figure B: Inter-peak intervals**
- X axis: AP number (1-13)
- Y axis: inter_peak values (inter_peak_1_to_2, inter_peak_2_to_3, ...)
- Plot type: Line
- Goal: show how firing rate changes across sequential APs

**Figure C: Voltage timeseries overlay**
- Load voltage_timeseries.csv (if parser handles it)
- Plot membrane potential vs. time
- Overlay multiple traces at different [Ca]_out

### For each figure attempt, check:
- [ ] Correct plot type selected
- [ ] X and Y columns assigned correctly
- [ ] Axis labels set (including units: mV, ms, mM)
- [ ] Title set
- [ ] Legend shows if multiple series
- [ ] Colors/theme applied (try "publication" theme)
- [ ] Export to SVG: `File > Export Figure > SVG`
- [ ] Export to PDF: `File > Export Figure > PDF`
- [ ] Exported file looks correct when opened externally

---

## Step 3 — Document friction

For EVERY point where the workflow breaks, is confusing, or requires
a workaround, fill in one entry:

### Friction Log Template

```
### F-<number>: <one-line summary>

**Severity**: BLOCKER / MAJOR / MINOR / COSMETIC
**Step**: (which step above)
**Expected**: (what you expected to happen)
**Actual**: (what actually happened)
**Workaround**: (if any)
**Screenshot**: (path or inline)
```

Examples of what to look for:
- Can't figure out how to change axis labels
- LaTeX math in labels doesn't render
- Export SVG has wrong fonts
- Can't overlay two datasets in one plot
- Color picker is missing
- Units aren't recognized
- Data table doesn't scroll to column I need
- Plot auto-range picks wrong bounds
- Theme doesn't affect axis labels

---

## Step 4 — Verdict

After completing Steps 1-3, fill in:

```
### Data loading
- CSV (columnar):  PASS / FAIL
- CSV (transposed): PASS / FAIL / EXPECTED_FAIL
- HDF5:            PASS / FAIL / NOT_TESTED

### Figure reproduction
- Could reproduce research figure: YES / PARTIAL / NO
- Export quality acceptable: YES / NO

### Friction count
- BLOCKER: <n>
- MAJOR: <n>
- MINOR: <n>
- COSMETIC: <n>

### Overall
- Ready for Phase 11: YES / YES_WITH_CAVEATS / NO
- Reason: <one sentence>
```

---

## Step 5 — File the results

Save the completed friction log to:
```
.lumen-ops/b2-friction-log.md
```

Commit with message:
```
docs: B2 real-data verification results

Co-Authored-By: Claude Opus 4.6 (1M context) <noreply@anthropic.com>
```

---

## Assistant support

While you (T.Y.) execute this protocol, the assistant can:
- Launch Lumen for you: `DISPLAY=:0 ./build/bin/lumen`
- Take screenshots at any step
- Help debug loader failures
- Help with axis label formatting (LaTeX syntax)
- Export figures on your behalf
- Fix BLOCKER-level friction items immediately

To start: say "launch" and open your first data file.
