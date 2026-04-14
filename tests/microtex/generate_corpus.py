#!/usr/bin/env python3
"""Generate the 60-equation golden corpus for MicroTeX verification.

Output: tests/microtex/corpus/tier{1,2,3}_NN.tex
Each file contains a single LaTeX math expression.
"""

import os
from pathlib import Path

OUT_DIR = Path(__file__).parent / "corpus"

# Tier 1: basic math (20 equations, 100% required)
TIER1 = [
    r"\alpha + \beta = \gamma",
    r"\sigma^2",
    r"x_{i} + y_{j}",
    r"\frac{a}{b}",
    r"\sqrt{x^2 + y^2}",
    r"\sqrt[3]{27}",
    r"\sum_{i=0}^{n} x_i",
    r"\int_0^\infty e^{-x} dx",
    r"\prod_{k=1}^{N} a_k",
    r"\partial f / \partial x",
    r"\nabla \cdot \mathbf{E} = \frac{\rho}{\epsilon_0}",
    r"E = mc^2",
    r"\Delta G = \Delta H - T\Delta S",
    r"\pm 1.96 \sigma",
    r"a \leq b \leq c",
    r"x \neq y, \quad x \approx z",
    r"\mathrm{Re}(z) + i\,\mathrm{Im}(z)",
    r"\hat{x} \cdot \hat{y} = 0",
    r"\vec{F} = m\vec{a}",
    r"\left( \frac{a}{b} \right)",
]

# Tier 2: annotation equations (30 equations, >=95% required)
TIER2 = [
    r"\overline{x} = \frac{1}{N}\sum_{i=1}^{N} x_i",
    r"\underline{\text{significant}}",
    r"\binom{n}{k} = \frac{n!}{k!(n-k)!}",
    r"\lim_{x \to 0} \frac{\sin x}{x} = 1",
    r"\sup_{x \in S} f(x)",
    r"\inf_{n \geq 1} a_n",
    r"\max(a, b, c)",
    r"\min_{x} \|Ax - b\|^2",
    r"\arg\max_{\theta} P(D|\theta)",
    r"A \Rightarrow B",
    r"f: X \to Y",
    r"x \mapsto x^2",
    r"a \Leftarrow b",
    r"\limsup_{n\to\infty} a_n",
    r"\liminf_{n\to\infty} b_n",
    r"V_m = -70\,\text{mV}",
    r"I_{\text{Na}} + I_{\text{K}} + I_{\text{L}} = C_m \frac{dV}{dt}",
    r"\tau_m = R_m C_m",
    r"g_{\text{Na}} \cdot m^3 h (V - E_{\text{Na}})",
    r"\chi^2 = \sum \frac{(O-E)^2}{E}",
    r"p < 0.05",
    r"r^2 = 0.95",
    r"\mu \pm \sigma",
    r"F = k \frac{q_1 q_2}{r^2}",
    r"\oint \mathbf{B} \cdot d\mathbf{l} = \mu_0 I",
    r"\mathbb{R}^n",
    r"\mathbb{Z} \subset \mathbb{Q} \subset \mathbb{R}",
    r"\mathcal{L}(x, \lambda) = f(x) + \lambda g(x)",
    r"\dot{x} = f(x, u)",
    r"\ddot{q} + \omega^2 q = 0",
]

# Tier 3: complex/unsupported (10 equations, best-effort)
TIER3 = [
    r"\frac{\partial^2 u}{\partial t^2} = c^2 \nabla^2 u",
    r"\int_{-\infty}^{\infty} e^{-x^2} dx = \sqrt{\pi}",
    r"\sum_{n=0}^{\infty} \frac{x^n}{n!} = e^x",
    r"\left| \frac{\partial(x,y)}{\partial(u,v)} \right|",
    r"\prod_{p \text{ prime}} \frac{1}{1-p^{-s}} = \sum_{n=1}^{\infty} n^{-s}",
    r"f(x) = \begin{cases} 1 & x > 0 \\ 0 & x = 0 \\ -1 & x < 0 \end{cases}",
    r"\begin{pmatrix} a & b \\ c & d \end{pmatrix}",
    r"\ce{H2O}",  # mhchem — unsupported
    r"\xrightarrow[\text{below}]{\text{above}}",  # best-effort
    r"\overbrace{a + b + c}^{\text{sum}}",
]


def write_corpus():
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    for i, eq in enumerate(TIER1, 1):
        path = OUT_DIR / f"tier1_{i:02d}.tex"
        path.write_text(eq + "\n")

    for i, eq in enumerate(TIER2, 1):
        path = OUT_DIR / f"tier2_{i:02d}.tex"
        path.write_text(eq + "\n")

    for i, eq in enumerate(TIER3, 1):
        path = OUT_DIR / f"tier3_{i:02d}.tex"
        path.write_text(eq + "\n")

    print(f"Generated {len(TIER1) + len(TIER2) + len(TIER3)} corpus files in {OUT_DIR}")


if __name__ == "__main__":
    write_corpus()
