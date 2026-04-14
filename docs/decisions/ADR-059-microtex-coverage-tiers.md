# ADR-059: MicroTeX coverage tiers + golden corpus rationale

## Status
Accepted (Phase 9.5.3)

## Context
"Percentage of LaTeX supported" is unmeasurable — LaTeX is Turing-
complete. A falsifiable metric is needed to evaluate MathRenderer's
coverage for scientific figure use cases.

## Decision
Three-tier coverage model driven by a 60-equation golden corpus.

### Tier 1 (100% required, 20 equations)
Covers ~95% of axis labels, legends, and titles in scientific
papers: Greek letters, sub/superscripts, fractions, integrals,
summations, operators, font commands, accents, delimiters.

### Tier 2 (≥95% required, 30 equations)
Covers annotation equations and more complex math: environments
(align, cases, matrix), overline/underline, binom, text in math,
operator names (lim, sup, inf, etc.), arrows.

### Tier 3 (best-effort, 10 equations)
Complex or specialized: partial differential equations, infinite
series, Euler product, cases environments, mhchem chemistry
(unsupported), xrightarrow (best-effort).

### Corpus source
- 20 Tier 1: standard math notation from AMS testmath.tex
- 30 Tier 2: caption equations from Nature, PRL, Neuron papers
- 10 Tier 3: complex expressions testing edge cases

### Pass criteria
Each equation rendered by MathRenderer is compared (when LaTeX
reference is available) via SSIM with threshold 0.85 (ADR-060).
For unit tests: non-null QImage output is sufficient.

## Consequences
- + Falsifiable metric: X/20 Tier 1, Y/30 Tier 2
- + Corpus covers real-world scientific usage
- + Tier 3 failures are documented, not blocking
- - Corpus selection is subjective (mitigated by sourcing from
  actual papers)
- - 60 equations is a sample, not exhaustive

## Alternatives considered
- **"% of LaTeX" metric**: Rejected. Unmeasurable.
- **User survey**: Rejected. Not automated, not reproducible.
- **No coverage metric**: Rejected. Can't assess quality.
