# ADR-069: Application-Level Smoke Test Infrastructure

**Status**: Accepted
**Date**: 2026-04-18
**Context**: Phase 10.5 — B1 blocker resolution

## Problem

838 unit tests passed while the 3D rendering pipeline produced a
black screen (Phase 8 review audit). Unit tests verify data
structures and algorithms but not application-level behavior:
"does the app launch, load data, and render visibly?"

## Decision

Add a smoke test suite using Catch2 + Qt Test that verifies 5
end-to-end scenarios at the application level:

| ID | Scenario | Key assertion |
|----|----------|---------------|
| S1 | App launch | Window visible, menus present, no stale text |
| S2 | Load sample data | Plot rendered (pixel variance > 10), 3D dock shown, data table populated, title correct |
| S3 | Theme switch | All 6 themes applied without crash |
| S4 | Export | SVG/PDF files produced, valid content |
| S5 | 3D interaction | Camera drag changes framebuffer (when GL available) |

### Pixel variance check

The core assertion is `hasVisualContent(QImage, minVariance)`:
converts each pixel to luminance, computes standard deviation across
the framebuffer, returns true if stddev > threshold (default 10.0
on 8-bit scale). This catches all-black, all-white, and single-color
framebuffers while accepting any image with actual rendered content.

Threshold 10.0 is conservative — even a single-color wireframe on
a contrasting background produces stddev > 50.

### Headless rendering

- `QT_QPA_PLATFORM=offscreen` for all tests (injected via CMake)
- `Qt::WA_DontShowOnScreen` on MainWindow to avoid expose event
  crashes in offscreen mode while still allowing `grab()`
- 3D GL tests gracefully skip pixel checks when
  `isGLInitialized()` is false (offscreen Mesa limitation)
- For full 3D coverage: use `xvfb-run` in CI

### Build structure

Extracted `lumen_app` static library from the main executable so
smoke tests can link against all app code without duplicating
source files. The `lumen` executable now links `lumen_app` + main.cpp.

## Consequences

- Every future phase close must pass all smoke tests
- Phase reviews can no longer claim "rendering works" without
  pixel-level evidence
- CI workflow runs smoke tests on every push/PR
- 3D pixel checks require Xvfb on CI (offscreen GL not available)
