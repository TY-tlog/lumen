#!/usr/bin/env python3
"""Render SVG to PNG using Inkscape CLI.

Usage: python3 inkscape_runner.py input.svg output.png [width]

Requires: apt install inkscape
"""

import subprocess
import sys


def render_svg(svg_path: str, png_path: str, width: int = 1050) -> None:
    subprocess.run(
        ["inkscape", svg_path,
         "--export-type=png",
         f"--export-filename={png_path}",
         f"--export-width={width}"],
        check=True, capture_output=True
    )


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} input.svg output.png [width]")
        sys.exit(1)

    w = int(sys.argv[3]) if len(sys.argv) > 3 else 1050
    render_svg(sys.argv[1], sys.argv[2], w)
