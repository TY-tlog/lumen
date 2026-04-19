# Test Failure Audit — 79 "pre-existing" failures

**Date**: 2026-04-18
**Auditor**: Claude (prompted by project owner T.Y.)

## Root cause

All 79 failures had a single root cause: **fontconfig ASan leak**.

Every test that touches `QFont`, `QPainter`, or `QFontMetrics` triggers
fontconfig initialization via Qt's `QFontconfigDatabase`. Fontconfig
allocates internal structures (`FcInit`, `FcFontRenderPrepare`,
`FcFontMatch`) that are never freed — a known, benign system library
behavior. ASan's LeakSanitizer detects this (320+ bytes) and exits
with a non-zero code, causing ctest to mark the test as Failed.

**All 79 tests passed their Catch2 assertions.** Zero actual test
failures, zero real bugs.

## Categorization

| Bucket     | Count | Description                              |
|------------|-------|------------------------------------------|
| **ENV**    | 79    | fontconfig/Qt ASan leak (system library)  |
| KNOWN_GAP  | 0     | —                                        |
| REAL_BUG   | 0     | —                                        |

## Affected test groups

| Group                         | Count | Function leaked from         |
|-------------------------------|-------|------------------------------|
| PlotScene/computeMargins      | 16    | FcInit (320 bytes)           |
| HitTester/hitTestPoint        | 13    | FcInit (320 bytes)           |
| PlotRenderer                  | 3     | FcInit (320 bytes)           |
| FigureExporter                | 9     | FcInit (320 bytes)           |
| FontEmbedder/Academic fonts   | 9     | FcInit (320 bytes)           |
| MathRenderer                  | 7     | FcFontRenderPrepare (61 KB)  |
| Tier1 MicroTeX                | 15    | FcFontRenderPrepare (61 KB)  |
| ContourPlot labels            | 1     | FcInit (320 bytes)           |
| TextAnnotation                | 1     | FcInit (320 bytes)           |
| PDF structure                 | 2     | FcInit (320 bytes)           |
| SVG export (ADR-055)          | 1     | FcInit (320 bytes)           |
| PlotScene title               | 2     | FcInit (320 bytes)           |

## Fix applied

1. Created `tests/lsan_suppressions.txt` with fontconfig suppressions
2. Modified `tests/CMakeLists.txt` to inject `LSAN_OPTIONS` via
   `TEST_INCLUDE_FILES` (cmake 3.28 `set_directory_properties`)
3. Added `DISCOVERY_MODE PRE_TEST` + `PROPERTIES ENVIRONMENT` to
   all three `catch_discover_tests()` calls
4. Added `QT_QPA_PLATFORM=offscreen` for headless test execution

## Result

- **Before**: 91% pass (79 of 832 marked Failed)
- **After**: 100% pass (832 of 832)
- **Effective test count**: unchanged (832)
- **No tests dropped, skipped, or disabled**

## CI verification

No GitHub Actions CI is configured for this repository (private repo,
no workflow files). All verification is local. When CI is added, the
suppression file and environment settings will carry over via CMake.
