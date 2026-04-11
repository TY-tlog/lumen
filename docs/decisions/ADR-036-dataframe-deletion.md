# ADR-036: DataFrame fully deleted, no legacy wrapper

## Status
Accepted (Phase 6)

## Context
Phase 6 replaces DataFrame with TabularBundle (a group of rank-1
Datasets). The question is whether to keep DataFrame as a
deprecated wrapper, maintain both APIs in parallel, or delete it
entirely.

## Decision
Delete DataFrame entirely. No wrapper, no deprecated API, no
parallel coexistence. All v1 code (275 tests, all callers) is
migrated to TabularBundle in Phase 6.2. After migration, the
DataFrame.h/.cpp and Column.h/.cpp files are removed from the
source tree.

## Consequences
- + Single API: no confusion about which data type to use
- + No maintenance burden for a deprecated wrapper
- + Forced migration ensures all code is on the new foundation
- + Cleaner architecture docs (no "legacy" section)
- - Largest migration in the project's history: every file that
  references DataFrame must be updated
- - Risk of introducing behavioral changes during migration.
  Mitigated by M6.2's byte-identical render regression test and
  v1 workspace load test.
- - No gradual deprecation: it's all-or-nothing in Phase 6.2.
  Acceptable because the project is small enough (275 tests,
  ~14K lines) to migrate in one sub-phase.

## Alternatives considered
- **Deprecated wrapper**: keep DataFrame as a thin wrapper around
  TabularBundle. Rejected: dual API is confusing, and the wrapper
  would need to be maintained indefinitely. Callers would never
  migrate voluntarily.
- **Parallel coexistence**: both DataFrame and TabularBundle exist
  indefinitely, used in different modules. Rejected: every
  cross-module boundary would need adapters. The complexity budget
  is better spent on clean migration.
