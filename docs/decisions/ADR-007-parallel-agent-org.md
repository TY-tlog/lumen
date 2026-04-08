# ADR-007: Parallel-agent organizational structure

## Status
Accepted (Phase 0); fully active from Phase 1.

## Context
Development uses multiple Claude Code agents in parallel, with the
owner acting as CEO/PO. Without strict boundaries the agents will
collide, overwrite each other, and produce inconsistent designs.

## Decision
Seven roles:

| Role         | Owns                                       |
|--------------|--------------------------------------------|
| Architect    | docs/specs, docs/plans, ADRs, design docs  |
| Backend      | src/lumen/{data,core,app,util}             |
| Frontend     | src/lumen/{ui,plot,style}                  |
| ML/Semantic  | (dormant until Phase 7)                    |
| QA           | tests/**                                   |
| Integration  | merging PRs, CHANGELOG, releases           |
| Docs         | README, docs site, CLAUDE.md sync          |

Each agent works in its own git worktree on its own branch.
Cross-role communication is asynchronous and file-based, via
`.lumen-ops/INBOX/` and `.lumen-ops/STATUS.md`. Merges into `main`
go through PR.

## Consequences
- + High parallelism, clear ownership
- + Trail of decisions and PRs
- - Significant upfront infrastructure
- - Agents must read their role file first

## Alternatives considered
- Single-agent serial development: slow
- Free-form multi-agent: chaos
