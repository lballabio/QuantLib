# Build and Test

Companion to [`AGENTS.md`](../AGENTS.md), which carries only the two commands
needed most often. This file holds the full set of build options, test
invocations, and CI entry points.

## 1. CMake (Recommended)

```bash
mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release \
    -DQL_BUILD_TEST_SUITE=ON -DQL_BUILD_EXAMPLES=ON
cmake --build .
```

Preset example (exists in `CMakePresets.json`):

```bash
cmake --preset linux-gcc-ninja-release
cmake --build build/linux-gcc-ninja-release
```

Key CMake options from `CMakeLists.txt`:

- `QL_BUILD_TEST_SUITE` default `ON`
- `QL_BUILD_EXAMPLES` default `ON`
- `QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER` default `OFF`
- `QL_COMPILE_WARNING_AS_ERROR` default `OFF`
- `QL_USE_STD_CLASSES` default `OFF`

Note: CI often overrides defaults (for example,
`QL_COMPILE_WARNING_AS_ERROR=ON` in CMake workflows/presets).

## 2. Autotools

```bash
./autogen.sh
./configure --with-boost-include=/path/to/boost
make -j 4
```

Frequently used configure flags (`configure.ac`):

- `--enable-unity-build`
- `--enable-intraday`
- `--enable-std-classes`
- `--enable-thread-safe-observer-pattern`
- `--enable-sessions`
- `--enable-openmp`
- `--enable-parallel-unit-test-runner`

## 3. Visual Studio / MSVC

- Open `QuantLib.sln` and build the desired config/platform, or use `msbuild`.
- Ensure the Boost include path is configured (for example via `Build.props`,
  see `.ci/VS2022.props`).

## 4. Running Tests

```bash
# CMake build tree (run from build/)
./test-suite/quantlib-test-suite --log_level=message

# Autotools build from repo root
./test-suite/quantlib-test-suite --log_level=message

# CMake after install may expose the binary in PATH
quantlib-test-suite --log_level=message

# Specific suite (run from build/)
./test-suite/quantlib-test-suite --log_level=message \
    --run_test=QuantLibTests/EuropeanOptionTests

# Single test case (run from build/)
./test-suite/quantlib-test-suite --log_level=message \
    --run_test=QuantLibTests/EuropeanOptionTests/testValues

# CTest path
ctest -V
```

## 5. CI Reality Check

Workflows are under `.github/workflows/`. Core validation files include:

- `linux.yml`, `macos.yml`, `msvc.yml`, `cmake.yml`
- `tidy.yml`, `headers.yml`, `filelists.yml`

For portability-sensitive changes, also check non-default matrices such as:

- `linux-nondefault.yml`, `linux-full-tests.yml`, `msvc-nondefault.yml`,
  `cmake-latest-runners.yml`

Automation workflows (e.g., generated headers/includes/namespaces/copyright)
also exist and can change over time; header and file-list touches should stay
green in `generated-headers.yml`, `includes.yml`, and `filelists.yml`.

## 6. Source of Truth

When any of the above looks stale, verify against:

- `.github/workflows/{linux,linux-nondefault,linux-full-tests,macos,msvc,msvc-nondefault,cmake,cmake-latest-runners}.yml`
- `CMakeLists.txt`, `CMakePresets.json`, `configure.ac`
- `test-suite/CMakeLists.txt`, `test-suite/Makefile.am`
- `.clang-format`, `.clang-tidy`
