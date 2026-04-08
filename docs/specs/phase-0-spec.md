# Phase 0 — Foundation

## Goal

Establish project skeleton, build system, conventions, and
inter-agent infrastructure. The result is a launchable empty Qt
window plus everything needed for the parallel agent organization
to start working in Phase 1.

## Active agents

- Architect
- Human (T.Y.)

All other agents are dormant until Phase 1.

## Deliverables

- [x] CMake 3.28+ project, Ninja generator, ccache integration
- [x] C++20, strict warnings, warnings-as-errors
- [x] Qt 6.6+ Core/Gui/Widgets linked
- [x] Catch2 v3 test harness via FetchContent
- [x] AddressSanitizer + UBSan opt-in via CMake options
- [x] clang-format and clang-tidy configurations
- [x] GitHub Actions CI matrix: Ubuntu 24.04 + macOS 14, Debug + Release
- [x] Directory layout per `docs/architecture.md`
- [x] Root `CLAUDE.md` and per-module `CLAUDE.md` stubs
- [x] `AGENTS/` with all 7 role definitions
- [x] `.lumen-ops/` message bus
- [x] ADRs 000–008 covering all Phase 0 decisions
- [x] `lumen` executable launches an empty `QMainWindow` titled "Lumen"
- [x] Window remembers geometry and state via `QSettings`
- [x] Menu bar: File / View / Help with About dialog
- [x] Smoke tests: Catch2 link, Qt link
- [x] `scripts/setup_worktrees.sh` for parallel agents

## Non-goals (deferred)

- Plot engine (Phase 2)
- CSV parser (Phase 1)
- Data model (Phase 1)
- Property inspector (Phase 5)
- Apple-mood QSS (Phase 1 in design-system form, Phase 2 in code)
- LLM integration (Phase 7+)

## Acceptance criteria

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DLUMEN_ENABLE_ASAN=ON -DLUMEN_ENABLE_UBSAN=ON
cmake --build build
cd build && ctest --output-on-failure
./build/bin/lumen   # opens window, About works, Cmd/Ctrl+Q quits cleanly
```

CI green on both Ubuntu and macOS, both Debug and Release.

## Risks

- **Qt installation friction**. Document both online installer and
  package manager paths. CI uses `jurplel/install-qt-action`.
- **clang-tidy false positives** with Qt MOC-generated code. Header
  filter regex restricts to `(src|include)/.*`.
- **macOS code signing** for distribution is deferred to Phase 10.

## Exit checklist

- [ ] All deliverables done
- [ ] CI green on main
- [ ] `docs/reviews/phase-0-review.md` written
- [ ] `docs/specs/phase-1-spec.md` drafted, awaiting human approval
- [ ] `docs/design/design-system.md` v0 (color tokens, typography
      scale, spacing scale) drafted
