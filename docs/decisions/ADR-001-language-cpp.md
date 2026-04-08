# ADR-001: Primary language is C++20

## Status
Accepted (Phase 0)

## Context
Lumen must feel like a native desktop program (Prism, MATLAB figure
window). The owner wants to avoid Python for stability and control
reasons. The owner also wants to write the plot engine from scratch
on top of a GUI framework.

## Decision
Modern C++20, restricted to a safe subset:
- Prefer `std::unique_ptr` / `std::make_unique`; raw owning pointers
  are forbidden except inside Qt's parent-child system
- No `new` / `delete` directly except via `std::make_unique` or Qt
  parent-child
- `std::array` / `std::vector` instead of C arrays
- `QString` for user-facing text
- RAII everywhere
- `-Wall -Wextra -Wpedantic -Werror`, clang-tidy, ASan, UBSan in CI

## Consequences
- + Native speed, native binary, single executable
- + Direct access to Qt without bindings
- + Tools (sanitizers, static analysis) are mature
- - Steeper learning curve than Python
- - More boilerplate
- - Slower iteration vs Python; mitigated by ccache and small
  modules

## Alternatives considered
- Python + PySide6: rejected by the owner for stability/control
  preference, even though it would be faster to develop
- Rust + Qt (cxx-qt): cxx-qt is still rough; smaller ecosystem;
  Rust GUI frameworks lack Qt's widget breadth
- Rust + Slint / Iced / egui: insufficient widget systems for a
  property-inspector-driven workflow
