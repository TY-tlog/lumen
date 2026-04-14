#!/usr/bin/env python3
"""Render PDF to PNG using pdftocairo.

Usage: python3 pdftocairo_runner.py input.pdf output.png [dpi]

Requires: apt install poppler-utils (provides pdftocairo)
"""

import subprocess
import sys
from pathlib import Path


def render_pdf(pdf_path: str, png_path: str, dpi: int = 150) -> None:
    # pdftocairo appends -1 to the output name for first page.
    stem = Path(png_path).stem
    out_dir = Path(png_path).parent

    subprocess.run(
        ["pdftocairo", "-png", "-r", str(dpi), "-singlefile",
         pdf_path, str(out_dir / stem)],
        check=True, capture_output=True
    )

    # pdftocairo produces <stem>.png
    produced = out_dir / f"{stem}.png"
    if produced != Path(png_path):
        produced.rename(png_path)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} input.pdf output.png [dpi]")
        sys.exit(1)

    d = int(sys.argv[3]) if len(sys.argv) > 3 else 150
    render_pdf(sys.argv[1], sys.argv[2], d)
