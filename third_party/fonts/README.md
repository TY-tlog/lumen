# Academic Fonts for Lumen Export (Phase 9)

Place TTF/OTF font files here. Lumen's FontEmbedder registers
these as built-in academic fonts for publication-grade export.

## Required fonts (all open-licensed)

| Font | License | Source |
|------|---------|--------|
| Computer Modern | Public domain (Donald Knuth) | CTAN / cm-unicode |
| Liberation Serif | SIL OFL 1.1 (Red Hat) | github.com/liberationfonts |
| Liberation Sans | SIL OFL 1.1 (Red Hat) | github.com/liberationfonts |
| Source Serif Pro | SIL OFL 1.1 (Adobe) | github.com/adobe-fonts/source-serif |

## Installation

On Ubuntu: `sudo apt install fonts-liberation fonts-cmu`
On macOS: `brew install font-liberation font-computer-modern`

If system fonts are installed, FontEmbedder detects them
automatically via QFontDatabase. Manual placement of .ttf/.otf
files in this directory is only needed for bundled distribution.
