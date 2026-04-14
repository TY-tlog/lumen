# ADR-058: PDF/A output intent gap and mitigation

## Status
Accepted (Phase 9.5.2)

## Context
PDF/A-2b requires an /OutputIntents declaration with an embedded
ICC profile (the "output intent" specifying the target color
space). Qt's QPdfWriter does not natively emit /OutputIntents.

Phase 9 added ICC profile embedding via FigureExporter::Options::
iccProfileData, which sets QColorSpace on QImage for PNG. For PDF,
the profile bytes are available but QPdfWriter has no API to inject
/OutputIntents into the PDF stream.

## Decision
Accept the gap for Phase 9.5. QPdfWriter-generated PDFs are
structurally valid PDF 1.4 but NOT PDF/A-2b compliant due to
the missing /OutputIntents declaration.

### Mitigation path (Phase 10+)
1. Post-process: use a tool like `qpdf` or `pikepdf` (Python) to
   inject /OutputIntents into the finished PDF. This can be done
   as a FigureExporter post-step.
2. Alternative: replace QPdfWriter with a direct PDF generation
   library (e.g., libharu) that supports /OutputIntents natively.

### Current state
- PDF structure is valid (starts with %PDF-, has pages, %%EOF)
- ICC profile bytes are available via ColorProfile::iccBytes()
- PDF/A-2b validation via verapdf will fail on /OutputIntents
  check — this is expected and documented

## Consequences
- + Honest documentation of the gap
- + PDF export still works for most use cases (screen, print)
- + ICC profile infrastructure is in place for Phase 10 fix
- - Journal systems requiring strict PDF/A may reject Lumen PDFs
- - verapdf CI check will initially report this as a known failure

## Alternatives considered
- **Raw PDF byte manipulation**: Inject /OutputIntents by parsing
  and rewriting the QPdfWriter output. Rejected: fragile, error-
  prone, and would need to track QPdfWriter's internal structure.
- **Switch to libharu now**: Rejected: large dependency change
  mid-phase. Better to defer to Phase 10 when the style system
  redesign may already touch the export pipeline.
