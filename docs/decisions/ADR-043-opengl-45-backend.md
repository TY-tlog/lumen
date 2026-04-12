# ADR-043: OpenGL 4.5 backend on Phase 7 GPU layer

## Status
Accepted (Phase 8)

## Context
Phase 8 needs a 3D rendering pipeline. Phase 7 established a
QOpenGLWidget GPU layer in PlotCanvas (ADR-039). Phase 8
generalizes this to PlotCanvas3D, a full QOpenGLWidget with
OpenGL 4.5 core profile for modern shader-based rendering.

## Decision
Use OpenGL 4.5 core profile directly via Qt's QOpenGLFunctions_4_5_Core.

Features required: DSA (Direct State Access), compute shaders
(optional, for future), SSBO, instanced rendering (Scatter3D),
3D textures (volume rendering), FBO for off-screen rendering and
hit-test.

Fallback: on macOS where GL 4.5 may not be available (Apple
deprecated OpenGL, max is 4.1), detect at init and fall back to
4.1 with feature flags disabling DSA and compute. Document
limitations.

CI: Ubuntu runners have Mesa llvmpipe which supports GL 4.5 in
software. macOS runners have Apple's GL driver (4.1 max). Tests
that require GL 4.5 are skipped on macOS CI with appropriate
SUCCEED() markers.

## Consequences
- + Direct GL gives maximum control and performance
- + Qt's QOpenGLWidget handles context creation and compositing
- + Phase 7's GPU layer pattern validated; Phase 8 extends it
- + Instanced rendering enables 1M+ Scatter3D at 60 FPS
- - Apple deprecated OpenGL; macOS 4.1 fallback loses some features
- - No Vulkan/Metal path; acceptable for Lumen's scope
- - Software rendering in CI is slower but functional

## Alternatives considered
- **Qt3D**: Qt's built-in 3D framework. Rejected: active
  deprecation concerns, limited control over shader pipeline,
  opinionated scene graph conflicts with Lumen's PlotItem model.
- **Vulkan via QRhi**: Qt's rendering hardware interface. Future-
  proof but significantly more complex (descriptor sets, pipeline
  objects, synchronization). Rejected for Phase 8; consider for
  Phase 12+ if GL limitations become blocking.
- **Platform-native split** (Metal on macOS + Vulkan on Linux):
  two rendering backends to maintain. Rejected: development cost
  prohibitive for one-person project.
