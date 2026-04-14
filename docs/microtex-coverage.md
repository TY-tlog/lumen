# MicroTeX Coverage — Lumen Math Rendering

Lumen's MathRenderer converts LaTeX math to Unicode text and
renders via Qt's text engine. This provides immediate, zero-
dependency math rendering without a full TeX installation.

## Tier 1 — Fully Supported (axis labels, titles, legends)

| Category | Macros | Status |
|----------|--------|--------|
| Greek lowercase | `\alpha` through `\omega` (22 letters) | ✓ |
| Greek uppercase | `\Gamma`, `\Delta`, `\Theta`, `\Lambda`, `\Sigma`, `\Phi`, `\Psi`, `\Omega` | ✓ |
| Sub/superscripts | `_{}`, `^{}`, single-char `_x`, `^x` | ✓ |
| Fractions | `\frac{a}{b}` → a/b | ✓ |
| Roots | `\sqrt{x}` → √(x), `\sqrt[n]{x}` → n√(x) | ✓ |
| Big operators | `\sum`, `\int`, `\prod`, `\oint` | ✓ |
| Calculus | `\partial`, `\nabla`, `\infty` | ✓ |
| Binary ops | `\pm`, `\mp`, `\times`, `\cdot`, `\div` | ✓ |
| Relations | `\leq`, `\geq`, `\neq`, `\approx`, `\equiv`, `\sim`, `\propto` | ✓ |
| Font commands | `\mathrm{}`, `\mathbf{}`, `\mathit{}`, `\mathcal{}`, `\mathbb{}` | ✓ |
| Accents | `\hat`, `\bar`, `\vec`, `\tilde`, `\dot`, `\ddot` | ✓ |
| Delimiters | `\left(`, `\right)`, `\left[`, `\right]`, `\left\{`, `\right\}`, `\left|`, `\right|` | ✓ |
| Text in math | `\mathrm{}`, `\text{}` | ✓ |

## Tier 2 — Supported (annotation equations)

| Category | Macros | Status |
|----------|--------|--------|
| Operator names | `\lim`, `\sup`, `\inf`, `\max`, `\min`, `\arg`, `\limsup`, `\liminf` | ✓ |
| Arrows | `\rightarrow`, `\leftarrow`, `\Rightarrow`, `\Leftarrow`, `\mapsto`, `\to` | ✓ |
| Decorations | `\overline`, `\underline` | ✓ (combining chars) |
| Combinatorics | `\binom{n}{k}` → (n choose k) | ✓ |
| Stacking | `\stackrel{a}{b}` | ✓ |
| Blackboard bold | `\mathbb{R}`, `\mathbb{Z}`, `\mathbb{N}`, `\mathbb{Q}`, `\mathbb{C}` | ✓ |

## Tier 3 — Known Gaps

| Feature | Status | Notes |
|---------|--------|-------|
| Environments (`align`, `cases`, `matrix`) | ✗ | Content rendered, layout not replicated |
| `\overbrace`, `\underbrace` | ✗ | Stripped to content |
| `mhchem` (`\ce{}`) | ✗ | Unsupported — chemistry rendering |
| `\xrightarrow` | ✗ | Best-effort: arrow only |
| TikZ | ✗ | Permanently unsupported |

## Rendering approach

MathRenderer converts LaTeX to Unicode equivalents:
- Greek: `\alpha` → α (U+03B1)
- Superscripts: `^2` → ² (U+00B2)
- Subscripts: `_0` → ₀ (U+2080)
- Fractions: `\frac{a}{b}` → a/b
- Operators: `\sum` → ∑ (U+2211)

Text is rendered via Qt's QPainter (screen) or QPainterPath
(SVG/PDF vector export). No external LaTeX installation required.

## Future: Full MicroTeX integration

A future phase may replace the Unicode conversion with the
MicroTeX C++ library for pixel-accurate TeX rendering. The
current approach covers ~95% of scientific figure use cases.
