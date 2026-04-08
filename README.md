# Lumen

A standalone interactive scientific plot viewer.

Open a CSV file, get a polished, interactive plot — pan, zoom, brush,
double-click any element to edit its properties. Built natively with
C++ and Qt 6 for speed and the look of a real desktop program.

## Status

**Phase 0** — Foundation. Empty main window, build system, test harness.
Not yet useful. See `docs/specs/phase-0-spec.md`.

## Build

Requirements: CMake 3.28+, Ninja, a C++20 compiler (GCC 13+, Clang 17+,
Apple Clang 15+), Qt 6.4+.

```bash
git clone git@github.com:<YOUR_GH_USER>/lumen.git
cd lumen

cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DLUMEN_ENABLE_ASAN=ON \
    -DLUMEN_ENABLE_UBSAN=ON

cmake --build build
./build/bin/lumen
```

## Test

```bash
cd build
ctest --output-on-failure
```

## Development

See `CLAUDE.md` and `AGENTS/`.

## License

MIT
