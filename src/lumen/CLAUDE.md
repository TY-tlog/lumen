# src/lumen — package root

This is the Lumen C++ source. Submodules:
- `app/` — Application class, startup, lifecycle
- `ui/` — main window, docks, dialogs (Frontend)
- `plot/` — self-built plot engine over QPainter (Frontend, Phase 2+)
- `data/` — CSV parser and data model (Backend, Phase 1+)
- `core/` — event bus, command bus, registries (Backend, Phase 1+)
- `style/` — design system, QSS, palette (Frontend)
- `util/` — small helpers shared across modules

See root `CLAUDE.md` for hard rules.
