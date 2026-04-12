# ADR-044: Camera mode selection — Trackball + Orbit

## Status
Accepted (Phase 8)

## Context
3D plot navigation requires camera control. Different scientific
workflows prefer different navigation models: crystallographers
prefer trackball (free rotation to inspect structure), microscopists
prefer orbit (constrained azimuth/elevation around specimen).

Phase 7 established the "user-selectable mode" pattern with 3-mode
reactivity. Phase 8 extends this originality to camera control.

## Decision
Two camera modes, user-selectable per PlotCanvas3D:

**Trackball** (default): arcball rotation using quaternion math.
Left-drag rotates freely in 3D. No gimbal lock. Best for
inspection of 3D structures where arbitrary orientations matter.

**Orbit**: constrained to azimuth (horizontal) and elevation
(vertical) angles around a target point. Left-drag changes
azimuth/elevation. Cannot roll. Best for scientific visualization
where "up" should always be up.

Both modes share: wheel zoom (dolly toward/away from target),
middle-button pan (translate target in screen plane).

Mode persisted in workspace file (Camera JSON includes mode).

## Consequences
- + User picks the navigation model matching their workflow
- + Extends the "selectable mode" originality pattern from Phase 7
- + Both modes are well-understood in scientific visualization
- - Two interaction code paths to maintain (manageable: ~100 lines each)
- - Mode switch may disorient user momentarily (acceptable)

## Alternatives considered
- **Trackball only**: simpler but frustrating for users who want
  constrained navigation. Rejected.
- **Orbit only**: simpler but frustrating for users who need free
  rotation (e.g., inspecting a molecule from all angles). Rejected.
- **Three modes including First-person** (WASD + mouse look): not
  a scientific use case. First-person is for games and architectural
  walkthroughs. Rejected.
