# Phase 9.5.2 Review — SVG/PDF Strict Compliance

**Date**: 2026-04-14
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-9.5-spec.md` §4
**Plan**: inline (small scope, no separate plan needed)

---

## What shipped

SVG 1.1 and PDF structural compliance test suites integrated
into the vector-consistency CI workflow. PDF/A-2b /OutputIntents
gap documented via ADR-058.

### SVG compliance (test_svg_compliance.py)
- Source fixture JSON validation (12 fixtures)
- Rendered SVG well-formed XML check
- Duplicate ID detection
- SVG namespace and viewBox requirement documentation

### PDF/A compliance (test_pdfa_compliance.py)
- PDF header (%PDF-) and footer (%%EOF) validation
- /Type and /Pages structure checks
- Source fixture precondition tests

### ADR-058
- QPdfWriter cannot emit /OutputIntents (PDF/A-2b requirement)
- ICC profile bytes available via ColorProfile::iccBytes()
- Mitigation: post-process with pikepdf in Phase 10

---

## Per-deliverable verification

- verified: test_svg_compliance.py runs and passes fixture validation (12/12 source fixtures valid JSON)
- verified: test_pdfa_compliance.py runs and passes fixture validation (12/12 source fixtures exist)
- verified: CI workflow extended with SVG and PDF/A compliance steps
- verified: ADR-058 documents PDF/A gap with concrete mitigation path
- verified: 765/765 C++ tests pass unchanged (zero regression)

---

## Exit checklist
- [x] SVG compliance test suite committed and integrated in CI
- [x] PDF/A compliance test suite committed and integrated in CI
- [x] ADR-058 documents /OutputIntents gap
- [x] CI uploads validator output as artifact on failure
- [x] 765 C++ tests pass unchanged
- [x] This review in SAME commit as STATUS close
