# AGENTS.md — AI Agent Guide for QuantLib

> How AI coding agents should work in this codebase.

## 1. Hard Constraints

Two things a change has to respect, neither of which is obvious from the file it
touches:

- **C++17 is the baseline, not a floor to build on.** `CMakeLists.txt` defaults
  `CMAKE_CXX_STANDARD` to 17 and `configure.ac` checks for it. CI additionally
  compiles under newer language modes up to C++26, so a post-C++17 construct
  will pass some matrices and fail the default build.
- **Three build systems are exercised in CI** — CMake, Autotools, *and* a Visual
  Studio solution. A new source file has to be registered in all of them, or
  `filelists.yml` fails on the pull request. See
  [`.agents/registering-new-files.md`](.agents/registering-new-files.md).

## 2. Task Guides

Task-specific detail lives in [`.agents/`](.agents/README.md). Read one when the
task calls for it:

| Read this | When |
| --- | --- |
| [`.agents/architecture.md`](.agents/architecture.md) | You need the instrument/engine lifecycle, the recalculation triggers, the cash-flow hierarchy, or the design patterns. |
| [`.agents/build-and-test.md`](.agents/build-and-test.md) | You need build options, configure flags, test invocations, or the CI workflow map. |
| [`.agents/registering-new-files.md`](.agents/registering-new-files.md) | You add, rename, or remove a `.hpp`/`.cpp` file. |
| [`.agents/extending-quantlib.md`](.agents/extending-quantlib.md) | You add a new instrument, term structure, calendar, or day counter. |
| [`.agents/deprecation.md`](.agents/deprecation.md) | You deprecate, change, or remove a public API. |
| [`.agents/maintaining-agent-docs.md`](.agents/maintaining-agent-docs.md) | You edit this file or anything in `.agents/`. |

## 3. Coding Conventions

### 3.1 Formatting and Includes

Source of truth: `.clang-format`.

- 4-space indent, no tabs.
- 100-column limit.
- Namespace indentation enabled.
- Pointer/reference alignment: `T* p`, `T& x`.
- Include order: local (`"..."`) → `<ql/...>` → `<boost/...>` → standard
  headers.

### 3.2 Naming and API Style

- Types/classes: `PascalCase`
- Functions/methods: `lowerCamelCase`
- Data members: trailing underscore (`engine_`, `calculated_`)
- Macros: `QL_UPPER_CASE`
- Prefer `const` correctness and pass heavy objects by `const&`.

### 3.3 Memory and Ownership

Use the QuantLib portability aliases in `ql/shared_ptr.hpp`:

- `ext::shared_ptr<T>`
- `ext::make_shared<T>(...)`
- `ext::dynamic_pointer_cast<T>(...)`

Use `std::unique_ptr` for strict local ownership in implementations.

### 3.4 Error Handling

Use the `ql/errors.hpp` macros, not raw `throw`/`assert`:

- `QL_REQUIRE`
- `QL_ENSURE`
- `QL_FAIL`
- `QL_ASSERT`

### 3.5 Header Pattern

New headers should have:

- The standard QuantLib license block
- Doxygen `\file` and brief
- Include guard `quantlib_<name>_hpp`
- Self-contained includes

## 4. Build and Test — Quick Start

```bash
cmake --preset linux-gcc-ninja-release
cmake --build build/linux-gcc-ninja-release
./build/linux-gcc-ninja-release/test-suite/quantlib-test-suite \
    --log_level=message
```

Build options, Autotools and MSVC instructions, targeted test invocations, and
the CI workflow map are in
[`.agents/build-and-test.md`](.agents/build-and-test.md).

## 5. High-Impact Pitfalls

These mostly fail silently — a wrong number or undefined behavior rather than a
build error, so no compiler or test will point at them for you.

### 5.1 Global Settings

- `Settings::evaluationDate()` is global by default; with sessions enabled it
  becomes per-session/per-thread.
- Test cases do **not** need their own `SavedSettings`. `TopLevelFixture`
  (`test-suite/toplevelfixture.hpp`) already holds one, so settings are restored
  after every case.

### 5.2 Lazy Evaluation Surprises

`Instrument::NPV()` returns a cached result and re-runs the pricing engine only
after an observed dependency has notified a change. Most of the non-obvious
behavior in this library follows from that:

- **Result accessors do not recompute on demand.** `NPV()` and its siblings
  return whatever the last `performCalculations()` cached. When a changed input
  does not move a price, the cause is almost always a missing notification, not
  a wrong formula.
- **Anything a price depends on has to arrive as an observable.** A data member
  holding a value copied at construction will never invalidate the cache; take a
  `Handle<Quote>` or a term-structure handle and `registerWith()` it.
- Notification forwarding behavior can differ (default/per-object, compile-time
  macro influence).
- Cycle handling can be silent unless `QL_THROW_IN_CYCLES` is enabled.
- Misused `freeze()` can leave stale values.

Before changing anything that touches caching, notification, or curve
construction, read [`.agents/architecture.md`](.agents/architecture.md).

### 5.3 Handles and Lifetimes

- `Handle<T>` defaults to `registerAsObserver=true`.
- If a handle does not own the pointee safely, observer callbacks can outlive
  valid memory.
- Relinking a `RelinkableHandle` can trigger broad recalculation cascades.

### 5.4 Dates and Conventions

- 30/360 variants differ materially.
- Actual/Actual ISMA requires schedule context for irregular periods.
- `Calendar::isEndOfMonth()` is business-day aware, not raw month-end.

### 5.5 Numerical Robustness

- Prefer robust solvers (often Brent) when derivatives are unavailable or noisy.
- Set realistic tolerances and max evaluations.
- Validate implied-vol and calibration outputs against known references.

### 5.6 Type Alias Nuance

`ql/types.hpp` aliases are macro-backed (`QL_REAL`, `QL_INTEGER`,
`QL_BIG_INTEGER` in `ql/qldefines.hpp`) and configurable; do not hard-code
assumptions beyond the defaults.

## 6. Validation Checklist

Before finishing a change:

- [ ] Build passes with no new warnings.
- [ ] Relevant tests pass.
- [ ] New behavior has tests.
- [ ] New price inputs are observables the object registered with, and a test
      shows that changing one moves the result (see 5.2).
- [ ] Build lists are updated where needed (`ql/*`, `test-suite/*`).
- [ ] Visual Studio project/filter files are updated for new files
      (`QuantLib.vcxproj*`, `test-suite/testsuite.vcxproj*`).
- [ ] Headers are self-contained.
- [ ] Error handling uses `QL_*` macros.
- [ ] Pointer types use `ext::shared_ptr` conventions where appropriate.
- [ ] Numerical tolerances are justified for quant outputs.
