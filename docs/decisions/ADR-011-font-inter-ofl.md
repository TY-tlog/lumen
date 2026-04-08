# ADR-011: Application font is Inter, licensed under OFL

## Status
Accepted (Phase 1)

## Context
The design system (ADR-008) specifies "Inter" as the primary font
on Linux, with SF Pro as the macOS system font. We need to bundle
Inter with the application so it renders consistently regardless
of the user's system fonts.

Inter is a typeface designed by Rasmus Andersson specifically for
computer screens at small sizes. It is widely used in developer
tools and scientific applications.

## Decision
Bundle Inter font files in `resources/fonts/`:
- `Inter-Regular.ttf`
- `Inter-Medium.ttf`
- `Inter-SemiBold.ttf`
- `Inter-Bold.ttf`
- `OFL.txt` (SIL Open Font License 1.1)

Load via `QFontDatabase::addApplicationFont()` at startup. Set as
the application-wide default font. On macOS, prefer the system
font (`-apple-system`) and use Inter as fallback only.

Font files are embedded via `qt_add_resources` so they are compiled
into the binary — no external file dependency at runtime.

## Consequences
- + Consistent, beautiful typography across all Linux installs
- + OFL license permits bundling, modification, redistribution
  with no restrictions beyond keeping the license file
- + Binary grows by ~400 KB (4 weight files) — negligible
- + No network fetch at runtime
- - Must update manually if Inter releases a new version (rare;
  Inter is mature and stable)
- - On macOS, Inter may feel slightly different from native SF Pro;
  mitigated by preferring system font on macOS

## Alternatives considered
- **Rely on system-installed Inter**: not guaranteed to be present
  on user machines; inconsistent rendering.
- **Use Noto Sans**: good alternative, but Inter has better screen
  optimization at small sizes and is more Apple-mood aligned.
- **Use SF Pro on all platforms**: SF Pro license restricts usage
  to Apple platforms only. Cannot legally bundle on Linux.
- **Use Qt's default font**: inconsistent across distros; does not
  match the Apple-mood design system.
