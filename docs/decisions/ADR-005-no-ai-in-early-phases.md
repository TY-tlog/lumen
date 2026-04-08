# ADR-005: AI / agent / semantic layer deferred to Phase 7+

## Status
Accepted (Phase 0)

## Context
Earlier discussions imagined an AI-augmented viewer with LLM-driven
schema inference, plot recommendations, and a tool-using agent.
Recent clarification narrowed the goal: the owner primarily wants a
MATLAB-figure-class standalone viewer. The AI features are
desirable but not central, and they expand scope dramatically.

## Decision
Defer all AI / LLM work to Phase 7 or later. Phases 0–6 build a
complete viewer with no LLM dependency. The architecture leaves a
clean seam for adding an `lumen::ai` module later, but ships
nothing AI-related until then.

## Consequences
- + Phase 0–6 fits in 8–14 months instead of 18–24
- + The first usable build is months earlier
- + The owner can dogfood the viewer without AI
- - LLM integration design happens later, with less learning time
- - If AI turns out to be central, we re-prioritize then

## Alternatives considered
- Build AI from Phase 4 onward: rejected; not what the owner wants
  first.
- Drop AI entirely: rejected; keep the door open.
