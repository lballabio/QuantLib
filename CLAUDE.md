# CLAUDE.md — QuantLib Codebase Guide for AI Assistants

## Project Overview

QuantLib is a comprehensive open-source C++ library for quantitative finance (modeling, trading, and risk management). Current version: **1.38-dev**. Licensed under BSD 3-Clause. Repository: https://github.com/lballabio/QuantLib

---

## Directory Structure

```
QuantLib/
├── ql/                     # Main library source code
│   ├── cashflows/          # Coupon types, cash flow handling, pricing
│   ├── currencies/         # Currency definitions (Africa, America, Asia, Europe, Oceania, Crypto)
│   ├── experimental/       # 28+ experimental/advanced feature directories
│   ├── indexes/            # Interest rate, equity, commodity, inflation indexes
│   ├── instruments/        # Bond, swap, option, forward class definitions
│   ├── legacy/             # Deprecated code kept for compatibility
│   ├── math/               # Matrices, optimization, distributions, interpolations, FFT
│   ├── methods/            # Finite differences, lattices, Monte Carlo
│   ├── models/             # Equity, short rate, market, volatility models
│   ├── patterns/           # Observable pattern, lazy evaluation, singleton
│   ├── pricingengines/     # 16 subdirectories for pricing different instruments
│   ├── processes/          # Stochastic processes (GBM, Heston, CIR, etc.)
│   ├── quotes/             # Market data handles
│   ├── termstructures/     # Yield curves, discount curves, volatility surfaces
│   ├── time/               # Date handling, calendars, day counters, schedules
│   └── utilities/          # General utilities, formatting, parsing
├── test-suite/             # Boost.Test unit tests (~250+ test files)
├── Examples/               # 18 example applications
├── Docs/                   # Doxygen configuration and doc pages
├── cmake/                  # CMake utility modules
├── m4/                     # GNU Autoconf m4 macros
├── .github/workflows/      # 26+ GitHub Actions CI/CD workflows
└── .ci/                    # Visual Studio property files (VS2017–VS2022)
```

### Key `ql/pricingengines/` Subdirectories
`asian/`, `barrier/`, `basket/`, `bond/`, `capfloor/`, `cliquet/`, `credit/`, `exotic/`, `forward/`, `inflation/`, `lookback/`, `quanto/`, `swap/`, `swaption/`, `vanilla/`

### Key `ql/models/` Subdirectories
- `equity/` — Black-Scholes, Heston, jump-diffusion
- `shortrate/` — Vasicek, CIR, Hull-White, G2++
- `marketmodels/` — LIBOR market model
- `volatility/` — Volatility smile and surface models

---

## Build System

QuantLib supports three build systems. **CMake is the preferred modern approach.**

### CMake (Recommended)

**Requirements:** CMake ≥ 3.15, C++17 or higher, Boost ≥ 1.30.2

```bash
# Standard out-of-source build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

**Key CMake options:**

| Option | Default | Description |
|--------|---------|-------------|
| `QL_BUILD_EXAMPLES` | ON | Build example programs |
| `QL_BUILD_TEST_SUITE` | ON | Build test suite |
| `QL_ENABLE_OPENMP` | OFF | Enable OpenMP parallelism |
| `QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER` | OFF | Parallel test execution |
| `QL_ENABLE_SESSIONS` | OFF | Session-based singleton instances |
| `QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN` | OFF | Thread-safe observers |
| `QL_EXTRA_SAFETY_CHECKS` | OFF | Additional runtime checks |
| `QL_HIGH_RESOLUTION_DATE` | OFF | Microsecond date resolution |
| `QL_USE_STD_CLASSES` | OFF | Enable all `QL_USE_STD_*` options |
| `QL_USE_STD_SHARED_PTR` | OFF | Use `std::shared_ptr` instead of Boost |
| `QL_USE_STD_OPTIONAL` | OFF | Use `std::optional` instead of Boost |
| `QL_USE_STD_ANY` | OFF | Use `std::any` instead of Boost |
| `QL_USE_CLANG_TIDY` | OFF | Run clang-tidy during build |
| `QL_ERROR_FUNCTIONS` | OFF | Include function name in error messages |
| `QL_ERROR_LINES` | OFF | Include file/line in error messages |
| `QL_FASTER_LAZY_OBJECTS` | ON | Forward only first notification |

**CMake presets** are available for Windows (MSVC, Clang-CL), Linux (GCC, Clang), and macOS. See `CMakePresets.json`.

### Autotools (Legacy)

```bash
./autogen.sh
./configure
make -j$(nproc)
```

### Visual Studio

Open `QuantLib.sln`. Use `QuantLib.props` to configure settings. The `.ci/` directory contains VS2017, VS2019, and VS2022 property files.

---

## Running Tests

Tests use the **Boost Unit Test Framework**. The test executable is `quantlib-test-suite`.

```bash
# Build and run all tests
cmake --build build --target quantlib-test-suite
./build/test-suite/quantlib-test-suite

# Run a specific test suite
./build/test-suite/quantlib-test-suite --run_test=Bonds

# Run with verbose output
./build/test-suite/quantlib-test-suite --log_level=message

# Run benchmark
./build/test-suite/quantlib-benchmark
```

Tests are organized by feature — one `.cpp` file per domain (e.g., `americanoption.cpp`, `bonds.cpp`, `swaps.cpp`). The global test fixture is in `test-suite/quantlibglobalfixture.cpp`.

---

## Code Conventions

### File Naming
- All filenames are **lowercase**, hyphen-separated for multi-word names (e.g., `asianoption.hpp`, `piecewiseyieldcurve.hpp`)
- Headers use `.hpp`, sources use `.cpp`
- Header and source files share the same base name

### Every Header File Must Have
1. **Emacs modeline** as the very first line:
   ```cpp
   /* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
   ```
2. **Copyright block** with author names and years
3. **Doxygen file doc** comment: `/*! \file filename.hpp \brief ... */`
4. **Include guard**: `#ifndef quantlib_filename_hpp` / `#define quantlib_filename_hpp`
5. **Closing endif comment**: `#endif`

### Namespace
- All public API lives in the `QuantLib` namespace
- Namespace contents are indented (enforced by clang-format: `NamespaceIndentation: All`)

### Naming Conventions
- **Classes**: PascalCase (e.g., `FixedRateBond`, `BlackScholesProcess`)
- **Methods/functions**: camelCase (e.g., `cleanPrice()`, `settlementDate()`)
- **Member variables**: trailing underscore (e.g., `coupon_`, `maturityDate_`)
- **Template parameters**: uppercase single letter or short PascalCase (e.g., `T`, `Traits`)

### Smart Pointers
- Use `ext::shared_ptr<T>` and `ext::make_shared<T>()` (abstracts over Boost/std depending on build options)
- Do **not** use `boost::shared_ptr` or `std::shared_ptr` directly in library code

### Error Handling
- Use `QL_REQUIRE(condition, message)` for precondition checks
- Use `QL_ENSURE(condition, message)` for postcondition checks
- Use `QL_FAIL(message)` to throw unconditionally
- These macros expand to `QuantLib::Error` exceptions

### Include Order (`.clang-format` enforced)
1. Local QuantLib headers (quoted, e.g., `"somelocal.hpp"`)
2. QuantLib library headers (`<ql/...>`)
3. Boost headers (`<boost/...>`)
4. Standard library headers (`<vector>`, `<string>`, etc.)

### Code Style (`.clang-format`)
- **Standard**: C++17
- **Based on**: LLVM style
- **Indent width**: 4 spaces (no tabs)
- **Column limit**: 100
- **Pointer alignment**: Left (`T& x`, not `T &x`)
- **Namespace indentation**: All levels indented
- **Short functions**: Inline only
- **Template declarations**: Always break before `<`

---

## Key Patterns and Abstractions

### Observable/Observer Pattern (`ql/patterns/`)
QuantLib's core design pattern. Classes inherit from `Observable` to notify dependents when data changes. Use `registerWith()` and `update()`. Thread-safe variant available via `QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN`.

### Lazy Evaluation (`ql/patterns/lazyobject.hpp`)
`LazyObject` subclasses defer computation until `calculate()` is called. Override `performCalculations()` — never call it directly, always call `calculate()` which handles the dirty flag.

### Handle/Quote System (`ql/handle.hpp`, `ql/quotes/`)
Market data is passed via `Handle<Quote>` — an observable, nullable smart pointer wrapper. Use `Handle<Quote>` for inputs to instruments/curves that must react to market data changes.

### Term Structures (`ql/termstructures/`)
Yield curves, vol surfaces, and credit curves all share the `TermStructure` base. They are typically constructed via bootstrap helpers and passed as `Handle<YieldTermStructure>` etc.

### Instrument Lifecycle
1. Create instrument with static data (terms, schedule)
2. Attach pricing engine via `instrument.setPricingEngine(engine)`
3. Call `instrument.NPV()` (triggers lazy recalculation)
4. Results are cached until market data changes

---

## Adding New Code

### New Instrument
1. Create `ql/instruments/myinstrument.hpp` and `.cpp`
2. Inherit from `Instrument` (or an appropriate subclass)
3. Override `setupArguments()`, `fetchResults()`, and `isExpired()`
4. Create a corresponding pricing engine in `ql/pricingengines/`
5. Add to `ql/instruments/all.hpp` (auto-generated; add source to `CMakeLists.txt`)
6. Add tests in `test-suite/myinstrument.cpp`

### New Pricing Engine
1. Create in the appropriate `ql/pricingengines/<category>/` subdirectory
2. Inherit from `GenericEngine<Arguments, Results>` or instrument-specific engine base
3. Override `calculate()`

### New Calendar or Day Counter
- Calendars: `ql/time/calendars/`
- Day counters: `ql/time/daycounters/`
- Register new implementations in the corresponding `all.hpp`

### Experimental Features
New features that are not yet production-ready go in `ql/experimental/<category>/`. These have looser API stability guarantees.

---

## CI/CD and Quality Checks

### GitHub Actions Workflows (`.github/workflows/`)

| Workflow | What it checks |
|----------|---------------|
| `linux.yml` | GCC/Clang builds on Ubuntu |
| `macos.yml` | Clang builds on macOS |
| `msvc.yml` | Visual Studio builds (VS2017–VS2022) |
| `cmake.yml` | CMake-specific configuration |
| `sanitizer.yml` | AddressSanitizer / UBSanitizer |
| `tidy.yml` | clang-tidy static analysis |
| `codeql-analysis.yml` | GitHub CodeQL security scanning |
| `coveralls.yml` | Code coverage (target: 80%) |
| `headers.yml` | Header consistency validation |
| `includes.yml` | Include dependency checks |
| `filelists.yml` | Verifies build file lists are complete |
| `namespaces.yml` | Namespace consistency |
| `copyrights.yml` | Copyright header validation |
| `misspell.yml` | Spelling checks |
| `doxygen.yml` | Documentation generation |

### Build Matrix
- **Compilers**: GCC 9–14, Clang 10–19, MSVC (VS2017–VS2022)
- **C++ standards**: C++17, C++20, C++23
- **Platforms**: Linux (Ubuntu), macOS, Windows

### Static Analysis
- **clang-format**: Run `clang-format -i <file>` to auto-format. Style defined in `.clang-format`.
- **clang-tidy**: Config in `.clang-tidy`. Enable via `-DQL_USE_CLANG_TIDY=ON`.
- **LeakSanitizer**: Config in `.lsan.txt`.
- **MSVC analysis**: Rules in `.msvc-analysis.ruleset`.

---

## Documentation

- API docs generated by **Doxygen** from `Docs/quantlib.doxy`
- Use `/*! ... */` for Doxygen comments in headers
- Use `\brief`, `\param`, `\return`, `\note`, `\warning` Doxygen tags
- Narrative docs live in `Docs/pages/*.docs`
- Generated HTML output goes to `Docs/html/` (gitignored)

---

## Development Workflow

1. **Feature branches**: Always work on a feature branch, never directly on `master`
2. **Pull requests**: Submit via GitHub PR; CI runs automatically
3. **Commit messages**: Clear, descriptive, in imperative mood
4. **Adding files**: When adding `.cpp` files, also add them to:
   - `CMakeLists.txt` in the relevant subdirectory
   - `Makefile.am` in the relevant subdirectory
   - The `all.hpp` aggregation header for the subdirectory (if adding headers)
5. **Copyright**: Add your name and year to the copyright block in any new or substantially modified file
6. **No generated files**: Do not commit generated files (see `.gitignore`)

---

## Common Pitfalls

- **Do not** include `<ql/quantlib.hpp>` in library code — use specific includes
- **Do not** use `using namespace QuantLib;` in headers
- **Do not** use raw `boost::shared_ptr` — use `ext::shared_ptr`
- **Do not** call `performCalculations()` directly — call `calculate()`
- **Always** add new source files to both `CMakeLists.txt` and `Makefile.am`
- **Always** add new headers to the appropriate `all.hpp` aggregation file
- **Always** include the Emacs modeline in new files
- **Always** use `QL_REQUIRE`/`QL_ENSURE`/`QL_FAIL` macros instead of throwing directly

---

## Useful References

- Official website: https://www.quantlib.org
- API documentation: https://www.quantlib.org/docs.shtml
- Installation guide: https://www.quantlib.org/install.shtml
- Mailing lists: https://www.quantlib.org/mailinglists.shtml
- Issue tracker: https://github.com/lballabio/QuantLib/issues
- Contributing guide: `CONTRIBUTING.md`
- Change history: `News.md`, `ChangeLog.txt`
