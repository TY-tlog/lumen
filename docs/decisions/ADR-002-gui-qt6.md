# ADR-002: GUI framework is Qt 6 (LGPL)

## Status
Accepted (Phase 0)

## Context
We need a desktop GUI framework that runs on Linux and macOS, has a
rich widget set (docks, trees, dialogs, menus, property browsers),
mature OpenGL/Vulkan integration, and a native look-and-feel that
can be customized to an Apple-mood aesthetic.

## Decision
Qt 6.6+ under LGPL. Modules used in Phase 0: Core, Gui, Widgets.
Future phases may add Network, Concurrent, Test.

## Consequences
- + Industry-standard, 30+ years of refinement
- + LGPL is fine for our use; no commercial license required
- + Rich dock manager, tree views, and signal/slot system match the
  IDE-style architecture we want
- + High-quality QPainter API for the self-built plot engine
- + Excellent macOS native integration
- - Wheel/install size; mitigated by online installer
- - Wayland on Linux 24.04 has occasional rendering quirks; we
  document the `QT_QPA_PLATFORM=xcb` fallback
- - The default Qt look is not Apple-mood; we customize via QSS
  (see ADR-008)

## Alternatives considered
- GTK 4: weaker macOS support, less mature dock system
- wxWidgets: smaller, fewer modern affordances
- Native per-OS (Cocoa + GTK): two codebases, prohibitive for one
  developer
