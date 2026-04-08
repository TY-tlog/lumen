# Lumen — Claude Code Working Document

> Every Claude Code session in this repository must read this file first.

## What Lumen is

Lumen is a standalone, native desktop program for interactive
exploration of scientific data files. It targets the experience of a
MATLAB figure window: open a file, see the data, pan and zoom, and
double-click any plot element to edit its appearance. It is written in
modern C++ and Qt 6 and ships as a single native binary.

The first user is the project owner (T.Y.). The first data format is
CSV. AI / agent features are deferred to a much later phase.

## Hard rules — never violate

1. **Modern C++ only.** C++20. No C-style arrays, no `new`/`delete`
   except inside Qt's parent-child system, no raw owning pointers, no
   C-style casts. Prefer `std::unique_ptr`, `std::make_unique`, RAII.
2. **Qt ownership model.** When a `QObject` has a parent, the parent
   deletes it. Use this; do not manually delete parented objects.
3. **No raw `char*` strings.** Use `QString` for user-facing text and
   `std::string` / `std::string_view` for internal text.
4. **Type safety.** Treat warnings as errors. The build must compile
   with `-Wall -Wextra -Wpedantic -Werror` and pass clang-tidy.
5. **No external plot libraries.** The plot engine is written by us
   on top of QPainter. Qt itself is allowed; QCustomPlot, QWT, Qt
   Charts are not.
6. **No external CSV / data libraries.** We write our own CSV parser.
7. **Tests first.** A new module ships with unit tests. The build
   passes ASan and UBSan in Debug mode.
8. **Layering.** UI does not know about parsing. Parsing does not
   know about UI. Communication via core services and signals.
9. **No printf / fprintf.** Use `qDebug() / qInfo() / qWarning() /
   qCritical()` from `<QDebug>` and route to a structured log later.
10. **Apple-mood design.** Generous spacing, soft shadows, rounded
    corners, modern sans-serif typography, restrained color, smooth
    animation. Defined in `docs/design/design-system.md`.

## Project layout

```
lumen/
├── CLAUDE.md                       # ← you are here
├── README.md
├── CMakeLists.txt                  # root build
├── cmake/                          # CMake helper modules
├── .clang-format / .clang-tidy
├── AGENTS/                         # per-role agent instructions
├── docs/
│   ├── specs/
│   ├── plans/
│   ├── reviews/
│   ├── decisions/                  # ADRs
│   ├── design/                     # design system, mockups
│   └── architecture.md
├── src/lumen/
│   ├── main.cpp                    # entry point
│   ├── app/                        # Application class
│   ├── ui/                         # main window, docks, dialogs
│   ├── plot/                       # plot engine (Phase 2+)
│   ├── data/                       # CSV parser, data model (Phase 1+)
│   ├── core/                       # event bus, command bus (Phase 1+)
│   ├── style/                      # design system, QSS, palette
│   └── util/                       # small helpers
├── include/lumen/                  # public headers (mostly empty)
├── tests/
│   ├── unit/
│   ├── integration/
│   └── fixtures/
├── resources/                      # icons, fonts, QSS
├── third_party/                    # vendored deps (none in Phase 0)
├── .lumen-ops/                     # inter-agent message bus
└── scripts/
```

## Current phase

**Phase 0 — Foundation.** See `docs/specs/phase-0-spec.md`.
Only the Architect agent and the human (T.Y.) are active. All other
agents activate from Phase 1.

## How to build

```bash
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DLUMEN_ENABLE_ASAN=ON \
    -DLUMEN_ENABLE_UBSAN=ON
cmake --build build
./build/bin/lumen
```

## How to test

```bash
cmake --build build
cd build && ctest --output-on-failure
```

## How to lint / format

```bash
# Format all C++ files
find src tests -name '*.cpp' -o -name '*.h' | xargs clang-format -i

# Static analysis
clang-tidy -p build src/lumen/**/*.cpp
```

## How to find things

- **Why is X done this way?** → grep `docs/decisions/` for the ADR.
- **What does module Y do?** → read `src/lumen/Y/CLAUDE.md`.
- **Who owns module Y?** → see `AGENTS/README.md`.
- **What's blocking work?** → `.lumen-ops/BLOCKERS.md`.
- **What needs the human?** → `.lumen-ops/DECISIONS_NEEDED.md`.
