# ADR-008: Design system targets Apple mood

## Status
Accepted (Phase 0); detailed tokens in docs/design/design-system.md.

## Context
The owner wants Lumen to feel like a polished, modern Apple-style
app: generous spacing, soft shadows, rounded corners, restrained
color, modern sans-serif typography, smooth animation. Qt's default
look does not match this; we must build a design system on top.

## Decision
Define a design system in `docs/design/design-system.md` covering:

- Color tokens (light + dark), semantic naming
  (background.primary, surface.elevated, text.primary, accent, …)
- Typography scale (sizes, weights, line heights), default font
  Inter (Linux) and SF Pro (macOS, system)
- Spacing scale (4 / 8 / 12 / 16 / 24 / 32 / 48 / 64)
- Radii (4 / 8 / 12 / 16)
- Shadow tokens (sm / md / lg / xl)
- Motion tokens (duration, easing)

Implementation lives in `src/lumen/style/` with a generated QSS
file plus runtime constants. All widgets use tokens, never literal
colors or sizes.

## Consequences
- + Visual consistency across the entire app
- + Easy palette switching (light/dark, future themes)
- + Reviewable in one document
- - Up-front design work
- - QSS has limits; some effects (real blur, native vibrancy) need
  per-platform work

## Alternatives considered
- Use Qt default style: rejected, looks dated
- Adopt KDE Breeze or macOS native fully: rejected, not flexible
  enough for Lumen's identity
