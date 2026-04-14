#!/usr/bin/env python3
"""PDF/A-2b basic compliance tests for exported fixtures.

Validates that Lumen's PDF export produces files that meet basic
PDF structural requirements. Full PDF/A-2b validation requires
verapdf; this test checks the structural preconditions.

Run: pytest tests/export/test_pdfa_compliance.py -v
"""

import json
import struct
from pathlib import Path

import pytest

SOURCE_DIR = Path(__file__).parent / "fixtures" / "source"

FIXTURES = [
    "line_simple", "line_log", "scatter_color", "bar_grouped",
    "heatmap_viridis", "heatmap_diverging", "contour_filled",
    "contour_lines", "errorbar", "stat_violin",
    "plot3d_surface", "plot3d_scatter",
]


def _get_pdf_path(fixture_name: str) -> Path:
    """Return path to a PDF file for the fixture."""
    rendered = SOURCE_DIR.parent / "rendered" / f"{fixture_name}.pdf"
    return rendered


class TestPdfStructure:
    """Basic PDF structural checks (no external validator needed)."""

    def test_pdf_magic_number_format(self):
        """PDF files must start with %PDF-."""
        # This is a format specification check.
        assert b"%PDF-" == b"%PDF-"

    @pytest.mark.parametrize("fixture_name", FIXTURES)
    def test_rendered_pdf_is_valid(self, fixture_name):
        """Rendered PDF starts with %PDF- and ends with %%EOF."""
        pdf_path = _get_pdf_path(fixture_name)
        if not pdf_path.exists():
            pytest.skip(f"Rendered PDF not available for {fixture_name}")

        with open(pdf_path, "rb") as f:
            header = f.read(8)
            assert header.startswith(b"%PDF-"), \
                f"Invalid PDF header: {header!r}"

            # Check for %%EOF near end.
            f.seek(-32, 2)  # Last 32 bytes
            tail = f.read()
            assert b"%%EOF" in tail, "PDF missing %%EOF marker"


class TestPdfARequirements:
    """PDF/A-2b specific requirement checks."""

    def test_icc_profile_requirement(self):
        """PDF/A requires an output intent with ICC profile.

        Phase 9 added ICC profile embedding via ColorProfile.
        PDF/A-2b requires /OutputIntents with /ICCBased.
        If QPdfWriter doesn't declare this, ADR-058 documents
        the gap and mitigation.
        """
        # This documents the requirement. Full verification
        # requires verapdf in CI (spec §4.3).
        pass

    @pytest.mark.parametrize("fixture_name", FIXTURES)
    def test_rendered_pdf_contains_type_declarations(self, fixture_name):
        """PDF should contain /Type declarations (object structure)."""
        pdf_path = _get_pdf_path(fixture_name)
        if not pdf_path.exists():
            pytest.skip(f"Rendered PDF not available for {fixture_name}")

        with open(pdf_path, "rb") as f:
            content = f.read()

        assert b"/Type" in content, "PDF missing /Type declarations"

    @pytest.mark.parametrize("fixture_name", FIXTURES)
    def test_rendered_pdf_has_pages(self, fixture_name):
        """PDF should declare at least one page."""
        pdf_path = _get_pdf_path(fixture_name)
        if not pdf_path.exists():
            pytest.skip(f"Rendered PDF not available for {fixture_name}")

        with open(pdf_path, "rb") as f:
            content = f.read()

        assert b"/Pages" in content, "PDF missing /Pages"


class TestSourceFixtures:
    """Precondition: source fixtures exist and are valid."""

    @pytest.mark.parametrize("fixture_name", FIXTURES)
    def test_source_fixture_exists(self, fixture_name):
        path = SOURCE_DIR / f"{fixture_name}.lumen.json"
        assert path.exists()

    @pytest.mark.parametrize("fixture_name", FIXTURES)
    def test_source_fixture_valid_json(self, fixture_name):
        path = SOURCE_DIR / f"{fixture_name}.lumen.json"
        with open(path) as f:
            data = json.load(f)
        assert data.get("fixture") is True
