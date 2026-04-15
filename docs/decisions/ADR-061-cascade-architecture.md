# ADR-061: 4-level cascade architecture (last-write-wins)

## Status
Proposed (Phase 10.1)

## Context
Lumen needs a style system that lets users control visuals at
multiple levels: project-wide themes, per-plot presets, and
per-element overrides. The resolution model must be simple enough
that users can predict where an edit lands, yet powerful enough
for publication-grade control.

## Decision
4-level cascade with last-write-wins per property, no specificity.

```
resolved[property] = first non-nullopt in:
  1. element_override  (priority 4, highest)
  2. plot_instance     (priority 3)
  3. preset            (priority 2)
  4. theme             (priority 1, lowest)
```

Each level is a `Style` struct where every property is
`std::optional<T>`. Nullopt means "this level doesn't specify;
fall through." A property set at any level is overridden by the
same property set at any higher level.

### No CSS specificity
CSS specificity (inline > ID > class > element) creates a
complex mental model. Lumen's 4 levels are sufficient and map
directly to user intent:
- Theme = "what does my lab/journal use?"
- Preset = "what does this plot type look like?"
- Plot instance = "what does this specific plot look like?"
- Element = "what does this one series/axis/annotation look like?"

### CascadeTrace
The cascade resolver has a variant that records
`(property, value, source_level)` triples for the style
inspector dev tool. This is critical for debugging "why does
this element look like X?"

## Consequences
- + Simple mental model: higher level always wins
- + Predictable: user knows where their edit lands
- + Style inspector can show exactly which level each property
  comes from
- + No need for !important or specificity overrides
- - Less flexible than CSS (no contextual selectors)
- - "All matching in this plot" promotion writes to plot level,
  which may override preset-level values unexpectedly (mitigated
  by inspector visibility)

## Alternatives considered
- **CSS-like specificity**: Rejected. Complexity is high, user
  mental model is fragile, and Lumen's 4 levels cover the use
  cases without specificity computation.
- **Single-level override** (only per-element): Rejected. No
  project-wide consistency. Every element styled independently.
- **Inheritance tree** (element → series → plot → theme): Rejected.
  Tree depth creates ambiguity ("is this from the series node or
  the plot node?"). Flat 4-level is clearer.
