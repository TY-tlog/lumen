# Lumen Agent Organization

Lumen is built by a team of Claude Code agents working in parallel,
coordinated by the project owner (T.Y., acting as CEO/PO).

## Roles

| Role         | Owns                                       |
|--------------|--------------------------------------------|
| Architect    | docs/specs, docs/plans, ADRs, design docs  |
| Backend      | src/lumen/{data,core,app,util}             |
| Frontend     | src/lumen/{ui,plot,style}                  |
| QA           | tests/**                                   |
| Integration  | merging PRs, CHANGELOG, releases           |
| Docs         | README, docs site, CLAUDE.md sync          |
| ML/Semantic  | (dormant until Phase 7)                    |

## Phase 0 exception

Only Architect and Human are active in Phase 0. All others activate
in Phase 1.

## Communication

Append-only files in `.lumen-ops/`:
- `STATUS.md` — daily progress log
- `INBOX/<role>.md` — inbound requests
- `DECISIONS_NEEDED.md` — items requiring human decision
- `BLOCKERS.md` — currently blocked work

## Workflow

1. Architect updates `docs/plans/phase-N-plan.md`
2. Engineers read their role file + plan
3. Engineer works in own git worktree on own branch
4. Engineer writes spec → tests → implementation
5. PR titled `[<role>] <task>`, QA reviews
6. Integration merges
7. Docs updates user-facing docs

## Hard rules common to all agents

- Read your role file before any code change
- Stay inside your ownership boundary; file requests in others'
  inboxes if you need their work
- Tests come before implementation
- All changes must build cleanly with `-Wall -Wextra -Werror`,
  pass clang-tidy, ASan, UBSan
- Update STATUS.md at the end of each work block
