# Coverage Baseline

Coverage is an optional local baseline for development and review. It must not fail CI on percentage until the project explicitly adopts thresholds.

## C++ Coverage Build

Use a separate build directory so coverage flags do not contaminate normal developer builds:

```powershell
cmake -S . -B build-coverage -DBATTLE_BUILD_TESTS=ON -DBATTLE_ENABLE_COVERAGE=ON
cmake --build build-coverage --parallel
ctest --test-dir build-coverage --output-on-failure
```

On this Windows workspace, pin Ninja and the MinGW compiler so CMake does not select a Visual Studio generator without a C++ toolchain:

```powershell
cmake -S . -B build-coverage-mingw -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/mingw64/bin/c++.exe" -DBATTLE_BUILD_TESTS=ON -DBATTLE_ENABLE_COVERAGE=ON
cmake --build build-coverage-mingw --parallel
ctest --test-dir build-coverage-mingw --output-on-failure
```

When `gcovr` is installed and visible on `PATH`, CMake also exposes:

```powershell
cmake --build <coverage-build-dir> --target coverage
```

That target runs CTest and writes:

- `<coverage-build-dir>/coverage/index.html`
- `<coverage-build-dir>/coverage/coverage.xml`
- a console summary

## Current Baseline Policy

- Coverage reports are informational only.
- No minimum percentage is enforced.
- Generated coverage files and reports stay in the build directory and are not committed.
- Use coverage as a gap-finding tool alongside the Test Impact Matrix, not as a replacement for focused happy-path, negative, authority, and resource-bound tests.

## Tooling Notes

`BATTLE_ENABLE_COVERAGE` requires a GNU or Clang C++ compiler. On this Windows workspace, `gcov` is available through MinGW; `gcovr` may need to be installed separately before the `coverage` target exists.
