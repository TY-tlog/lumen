# ADR-033: Hybrid memory — in-memory below 100 MB, chunked LRU above

## Status
Accepted (Phase 6)

## Context
Scientific datasets range from kilobytes (CSV traces) to terabytes
(microscopy volumes). A single memory strategy fails: always
in-memory blows RAM on large files; always chunked adds overhead on
small files.

## Decision
MemoryManager singleton decides storage mode per Dataset:
- Estimated size < 100 MB → StorageMode::InMemory (full load)
- Estimated size ≥ 100 MB → StorageMode::Chunked (LRU cache)

Chunked mode: data is divided into chunks (e.g., 2D slices of a
3D volume). Chunks are loaded on demand and evicted via LRU when
the memory budget is exceeded.

Memory budget: default 4 GB, user-configurable (256 MB to system
RAM). Status bar shows current usage.

100 MB threshold is fixed (not configurable) to keep the UX simple.

## Consequences
- + Small files load instantly (no chunk overhead)
- + Large files don't crash the app (bounded memory)
- + User can tune budget to their hardware
- + LRU naturally keeps hot data cached
- - 100 MB threshold is a heuristic; some 90 MB files benefit from
  chunking and some 110 MB files fit comfortably in RAM. Acceptable.
- - Chunk size selection affects performance; Phase 6 uses simple
  heuristics (e.g., 2D slices for 3D data)

## Alternatives considered
- **Always in-memory**: simple but fails for files > available RAM.
  The project owner works with electrophysiology data (small) but
  Phase 7+ targets microscopy (large). Rejected.
- **Always chunked**: works for all sizes but adds latency for small
  files that could be fully loaded in <100ms. Rejected.
- **User-selected per file**: adds decision burden. Most users don't
  want to choose. Rejected.
