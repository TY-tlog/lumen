# ADR-049: ICC color management via lcms2

## Status
Proposed (Phase 9)

## Context
Publication-grade export requires ICC color management for print
accuracy. Screen colors (sRGB) must convert to CMYK for print
workflows. PNG needs iCCP chunks; PDF needs /ICCBased ColorSpace
declarations. A color management engine is required.

## Decision
Vendor LittleCMS (lcms2) via CMake FetchContent as a static
library. lcms2 is MIT-licensed, C99, cross-platform, and the
de facto open-source ICC implementation.

Built-in profiles:
- sRGB IEC61966-2.1 (screen default)
- Adobe RGB (1998) (wide-gamut display)
- Display P3 (Apple displays)
- CMYK US Web Coated (SWOP) v2 (North American print)
- CMYK FOGRA39 (European print)
- Gray Gamma 2.2 (grayscale)

Built-in profiles are compiled in as byte arrays (lcms2
cmsOpenProfileFromMem). User ICC files parsed via
cmsOpenProfileFromFile.

### Embedding
- **PNG**: iCCP chunk written via QImage metadata or raw chunk
  insertion after IHDR.
- **PDF**: /ICCBased ColorSpace in page resource dictionary.
  QPdfWriter extended to declare ICC profile stream.

### Vendoring
FetchContent with pinned tag (lcms2-2.16). No submodule. CMake
option LUMEN_VENDOR_LCMS2=ON (default ON). Build as OBJECT
library linked into lumen_export.

## Consequences
- + Accurate color for print and wide-gamut displays
- + Industry-standard ICC profiles
- + MIT license, no compatibility issues
- + Static linking, no runtime dependency
- + ~15s CI build time increase (acceptable)
- - Adds ~200KB to binary (lcms2 object code)
- - iCCP chunk insertion requires knowledge of PNG structure
  (mitigated: well-documented format)
- - PDF ICC embedding requires QPdfWriter extension or raw
  PDF stream writing (complexity in T2)

## Alternatives considered
- **No color management** (raw sRGB everywhere): Rejected.
  Print output has wrong colors. Unacceptable for publication.
- **sRGB-only** (embed sRGB profile but no conversion):
  Rejected. Print workflows require CMYK. Half-measure.
- **Platform color APIs** (ColorSync on macOS, ICM on Windows):
  Rejected. Non-portable. Lumen targets Ubuntu + macOS with
  identical behavior. lcms2 provides that.
- **Conan/vcpkg for lcms2**: Rejected. Lumen has zero package
  manager dependencies. FetchContent keeps the build self-
  contained.
