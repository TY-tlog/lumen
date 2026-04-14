#!/usr/bin/env python3
"""Render SVG to PNG using headless Chromium via Playwright.

Usage: python3 playwright_runner.py input.svg output.png [width] [height]

Requires: pip install playwright==1.49.0
          playwright install chromium
"""

import sys
from pathlib import Path


def render_svg(svg_path: str, png_path: str, width: int = 1050, height: int = 700) -> None:
    from playwright.sync_api import sync_playwright

    svg_url = Path(svg_path).resolve().as_uri()

    with sync_playwright() as p:
        browser = p.chromium.launch()
        page = browser.new_page(viewport={"width": width, "height": height},
                                device_scale_factor=2)
        page.goto(svg_url)
        page.wait_for_load_state("networkidle")
        page.screenshot(path=png_path, full_page=False)
        browser.close()


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} input.svg output.png [width] [height]")
        sys.exit(1)

    w = int(sys.argv[3]) if len(sys.argv) > 3 else 1050
    h = int(sys.argv[4]) if len(sys.argv) > 4 else 700
    render_svg(sys.argv[1], sys.argv[2], w, h)
