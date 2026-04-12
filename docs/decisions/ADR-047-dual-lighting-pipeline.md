# ADR-047: Dual lighting pipeline — Phong default + PBR opt-in

## Status
Accepted (Phase 8)

## Context
3D plot items need lighting for depth perception. The question is
which lighting model: classic Phong (simple, fast, familiar) or
physically-based rendering (PBR: Cook-Torrance, metallic/roughness
workflow, modern, publication-quality).

## Decision
Dual pipeline: Phong is the default per PlotItem3D, PBR is opt-in
via material attachment.

**Phong** (Phase 8.1): ambient + diffuse + specular per vertex/
fragment. Simple, fast, sufficient for most scientific visualization.
Default for all items. One directional light + ambient.

**PBR** (Phase 8.6): Cook-Torrance BRDF, GGX normal distribution,
Smith geometry shadowing, Fresnel-Schlick. Metallic/roughness
workflow. Multi-light. Optional image-based lighting (IBL) via
cubemap.

Each PlotItem3D can independently choose Phong or PBR via an
optional PbrMaterial attachment. If no material is set, Phong is
used. The toggle is per-item, not per-scene — one Surface3D can
be PBR while another is Phong.

## Consequences
- + Fast default (Phong) for casual use
- + Publication-quality option (PBR) when needed
- + Per-item choice matches Phase 7's per-item reactivity pattern
- + Phong shader compiles on all GL 4.1+ (macOS compat)
- - Two shader programs to maintain
- - PBR adds ~200 lines of GLSL
- - IBL cubemap adds texture management complexity

## Alternatives considered
- **Phong only**: simple but looks dated for publication. Missing
  modern expectation. Rejected.
- **PBR only**: modern but overkill for scatter plots where the
  user just wants to see points. PBR materials require metallic/
  roughness tuning that most scientists won't do. Rejected as
  default.
- **Unified PBR with Phong as preset**: mathematically, Phong is
  a special case of PBR (metallic=0, roughness=1). Could use one
  shader with Phong preset. Considered but rejected: the Phong
  shader is simpler (fewer uniforms, faster compilation, easier
  to debug) and the distinction is clearer for users.
