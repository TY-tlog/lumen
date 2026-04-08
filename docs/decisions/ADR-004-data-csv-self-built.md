# ADR-004: CSV parser is built in-house

## Status
Accepted (Phase 0)

## Context
Phase 1 needs CSV loading. Available C++ CSV libraries vary in
quality, and the owner prefers writing core code by hand. CSV is a
deceptively simple format with real edge cases (quoting, escapes,
encodings, line endings, type inference) that we must handle
correctly.

## Decision
Write our own CSV parser following RFC 4180 with reasonable
extensions:
- Quoted fields with escaped quotes
- Configurable delimiter (default `,`)
- UTF-8 (BOM-aware), Latin-1 fallback
- LF / CRLF / CR line endings
- Empty cells = nulls
- Type inference: int → double → ISO-8601 datetime → string
- Streaming row callback for large files

Implementation in `src/lumen/data/CsvReader.{h,cpp}` (Phase 1).

## Consequences
- + Total control, no surprises
- + Easy to extend with metadata extraction for the semantic layer
- - We carry the test burden for all edge cases
- - Performance must be measured (target: 100 MB CSV in < 5 s on
  the dev machine)

## Alternatives considered
- vincentlaucsb/csv-parser: good but external dependency, conflicts
  with the in-house principle
- Apache Arrow C++ CSV reader: heavyweight dependency
- Qt's QTextStream: not designed for CSV correctness
