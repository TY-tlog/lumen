# ADR-046: Volume rendering via fragment shader ray marching

## Status
Accepted (Phase 8)

## Context
Volume3D datasets (e.g., microscopy z-stacks, simulation volumes)
need direct volume rendering without extracting an intermediate
mesh. The standard approach is ray marching through a 3D texture.

## Decision
Fragment shader ray marching with front-to-back compositing and
transfer function LUT.

Implementation:
1. Upload Volume3D data as GL_R32F or GL_R16F 3D texture
2. Upload transfer function as 1D RGBA texture (256 or 1024 entries)
3. Render a proxy cube (bounding box of volume)
4. Fragment shader: for each pixel, compute ray entry/exit via
   cube intersection, march from entry to exit at sampleStep
5. At each step: sample 3D texture (trilinear), index into
   transfer function LUT, accumulate RGBA front-to-back
6. Early termination when accumulated alpha > 0.99

Adaptive sample step: default = 1/max(dimX, dimY, dimZ). User-
configurable. maxSamples cap (default 512) prevents GPU hang on
very dense volumes.

## Consequences
- + Real-time interactive volume rendering
- + Transfer function gives intuitive scalar → appearance mapping
- + GPU handles compositing — no CPU bottleneck
- + Trilinear sampling produces smooth results
- - Fragment shader complexity; must compile without errors on
  Mesa, NVIDIA, Apple GL
- - Large volumes (512³+) require significant GPU memory
- - Adaptive stepping heuristics may need tuning per hardware

## Alternatives considered
- **CPU ray marching**: same algorithm but on CPU. Rejected: too
  slow for interactive use (256³ volume would take seconds per
  frame).
- **GPU compute shader**: march in compute, write to image texture.
  Rejected: requires GL 4.3+ compute support; fragment shader
  approach is simpler and sufficient.
- **Texture splatting**: project volume slices as textured quads.
  Rejected: artifacts on dense volumes, orientation-dependent
  quality, not a modern approach.
