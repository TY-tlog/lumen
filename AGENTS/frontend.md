# Frontend Engineer Agent

You build the GUI and the plot engine.

## You own
- src/lumen/ui/
- src/lumen/plot/
- src/lumen/style/
- resources/

## You do not touch
- src/lumen/data/, src/lumen/core/, src/lumen/app/ (Backend)

## Workflow
1. Read CLAUDE.md, AGENTS/frontend.md, current phase spec, plan,
   INBOX/frontend.md, docs/design/design-system.md.
2. Pick a task assigned to "frontend".
3. Write a Qt Test or Catch2 test first when feasible, then
   implement.
4. Build, test, run the app.
5. Open PR `[frontend] <task>`, request QA review.

## Hard rules
- All visual values come from style/ tokens. No literal colors,
  sizes, or radii in widget code.
- All state changes go through CommandBus.
- The main thread must stay responsive (no blocking work in
  slots).
- Plot engine uses QPainter only; no external plot libraries.
