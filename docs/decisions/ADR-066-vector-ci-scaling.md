# ADR-066: Vector CI scaling — theme rotation per PR

## Status
Accepted (Phase 10.2)

## Context
Phase 9.5 has 12 fixtures × 3 viewers = 36 comparisons (~1 min).
Phase 10.2 adds 6 themes → 12 × 6 × 3 = 216 comparisons.
Running all 216 on every PR would take ~6 min (exceeds 5-min cap).

## Decision
**Theme rotation per PR, full matrix on tags.**

- **Per PR**: run 12 fixtures × 1 theme × 3 viewers = 36 comparisons.
  Theme selected by `hash(PR number) % 6`. Every PR tests a
  different theme; over 6 PRs all themes are tested.
- **On `vphase-*` tags**: run full matrix 12 × 6 × 3 = 216.
  This is the gate for releases.

### Implementation
`vector-consistency.yml` gains a `THEME` env var:
- PR trigger: `THEME=$(python3 -c "print(['lumen-light','lumen-dark','publication','colorblind-safe','presentation','print-bw'][${{ github.event.pull_request.number }} % 6])")`
- Tag trigger: matrix strategy over all 6 themes.

## Consequences
- + PR CI stays under 2 min (36 comparisons)
- + Full coverage at every release tag
- + Every theme tested within 6 PRs on average
- - A theme regression could land on main if the rotating theme
  doesn't cover it (mitigated: tag-gate catches before release)

## Alternatives considered
- **Full matrix every PR**: Rejected. 6× CI time, exceeds 5-min cap.
- **Random theme per PR**: Rejected. Non-deterministic; same theme
  could repeat for many PRs while another goes untested.
- **Parallel matrix with 6 runners**: Rejected. 6× CI cost.
