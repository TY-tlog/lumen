# ADR-025: Workspace file format (.lumen.json sidecar v1)

## Status
Accepted (Phase 4)

## Context
Phase 4.1 introduces edit persistence: the user edits plot
properties (line color, axis labels, title, legend, viewport),
closes Lumen, reopens the same CSV, and sees their edits restored.
The edits must be stored somewhere. The CSV itself must not be
modified (rule: user data is read-only).

## Decision
Store edits in a JSON sidecar file alongside the CSV:

- Filename: `<csv_basename>.lumen.json` in the same directory
  (e.g., `data.csv` → `data.lumen.json`)
- Format: JSON, human-readable, editable in any text editor
- Schema version 1 with a `"version": 1` field for future
  migration support

Schema v1 structure:
```json
{
  "version": 1,
  "csv_path": "data.csv",
  "plot": {
    "viewport": { "xmin": 0.0, "xmax": 700.0, ... },
    "title": { "text": "...", "fontPx": 17, "weight": 75 },
    "xAxis": { "label": "Time (ms)", "rangeMode": "auto", ... },
    "yAxis": { ... },
    "legend": { "position": "top_right", "visible": true },
    "series": [
      { "xColumn": 0, "yColumn": 1, "color": "#0a84ff",
        "lineWidth": 1.5, "lineStyle": "solid",
        "name": "voltage_mV", "visible": true }
    ]
  }
}
```

Series are identified by column index (not name) since the user
may have renamed them in the legend.

Auto-load: when a CSV is opened, WorkspaceManager checks for the
sidecar and applies it silently if found.

## Consequences
- + Human-readable: user can inspect or hand-edit the JSON
- + Portable: copy CSV + sidecar together, edits follow
- + No modification to user's CSV data
- + Version field allows future schema evolution
- + Lightweight: typically < 1KB
- - Extra file alongside each CSV (may clutter directories with
  many CSVs). Acceptable; the .lumen.json extension is distinctive.
- - Sidecar is orphaned if CSV is moved without it. Acceptable;
  csv_path field provides diagnostics.
- - No merge support if two users edit the same CSV's workspace.
  Acceptable for single-user tool.

## Alternatives considered
- **Embed metadata in CSV**: add a comment header or trailing
  metadata section. Rejected: modifies user data, violates
  CLAUDE.md rule that CSV parser is read-only. Also breaks other
  tools that parse the CSV.
- **Central workspace database** (e.g., SQLite in ~/.config/lumen/):
  rejected: breaks file portability. If user copies CSV to another
  machine, edits are lost. Sidecar travels with the file.
- **Binary format** (e.g., protobuf, MessagePack): rejected: not
  human-readable, harder to diagnose, requires additional
  dependency. JSON is available via Qt's built-in QJsonDocument.
