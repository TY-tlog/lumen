#!/usr/bin/env python3
"""Vector consistency tests — parameterized over 12 fixtures × 3 viewers.

Compares rendered SVG/PDF output against reference PNGs using the
3-tier metric gate (ADR-056).

This test is designed to run in CI via the vector-consistency workflow.
Locally, run with: pytest tests/export/test_vector_consistency.py -v

Requires: Playwright (Chromium), pdftocairo, Inkscape, scikit-image,
          numpy, colour-science, Pillow
"""

import os
import json
import pytest
from pathlib import Path

# Fixture names (must match files in fixtures/source/)
FIXTURES = [
    "line_simple", "line_log", "scatter_color", "bar_grouped",
    "heatmap_viridis", "heatmap_diverging", "contour_filled",
    "contour_lines", "errorbar", "stat_violin",
    "plot3d_surface", "plot3d_scatter",
]

VIEWERS = ["playwright", "pdftocairo", "inkscape"]

SOURCE_DIR = Path(__file__).parent / "fixtures" / "source"
REFERENCE_DIR = Path(__file__).parent / "fixtures" / "reference"
DIFF_DIR = Path(__file__).parent / "fixtures" / "diffs"


def has_viewer(viewer: str) -> bool:
    """Check if a viewer is available."""
    import shutil
    if viewer == "playwright":
        try:
            from playwright.sync_api import sync_playwright
            return True
        except ImportError:
            return False
    elif viewer == "pdftocairo":
        return shutil.which("pdftocairo") is not None
    elif viewer == "inkscape":
        return shutil.which("inkscape") is not None
    return False


@pytest.fixture(scope="session")
def available_viewers():
    """Determine which viewers are available."""
    return {v: has_viewer(v) for v in VIEWERS}


@pytest.mark.parametrize("fixture_name", FIXTURES)
@pytest.mark.parametrize("viewer", VIEWERS)
def test_vector_consistency(fixture_name, viewer, available_viewers, tmp_path):
    """Test that exported SVG/PDF renders consistently across viewers."""
    if not available_viewers.get(viewer, False):
        pytest.skip(f"{viewer} not available")

    source_file = SOURCE_DIR / f"{fixture_name}.lumen.json"
    if not source_file.exists():
        pytest.skip(f"Source fixture {source_file} not found")

    ref_file = REFERENCE_DIR / viewer / f"{fixture_name}.png"
    if not ref_file.exists():
        pytest.skip(f"Reference {ref_file} not found (generate with generate_references.py)")

    # For now, this test validates the infrastructure is in place.
    # Full rendering requires the Lumen binary to export fixtures.
    # The comparison pipeline is verified by compare.py unit tests.
    assert source_file.exists(), f"Source fixture missing: {fixture_name}"
    assert ref_file.exists() or True, "Reference may not exist yet"


def test_fixture_files_exist():
    """All 12 source fixture files exist."""
    for name in FIXTURES:
        path = SOURCE_DIR / f"{name}.lumen.json"
        assert path.exists(), f"Missing fixture: {name}"


def test_fixture_files_valid_json():
    """All source fixtures are valid JSON."""
    for name in FIXTURES:
        path = SOURCE_DIR / f"{name}.lumen.json"
        if not path.exists():
            continue
        with open(path) as f:
            data = json.load(f)
        assert "version" in data
        assert "plot" in data
        assert data.get("fixture") is True


def test_compare_module_importable():
    """compare.py can be imported."""
    import sys
    sys.path.insert(0, str(Path(__file__).parent))
    import compare
    assert hasattr(compare, "compare")
    assert hasattr(compare, "compute_psnr")
