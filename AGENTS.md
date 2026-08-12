# AGENTS.md — AI Agent Guide for QuantLib

> How AI coding agents should understand, navigate, and contribute to this
> codebase.

## 1. Project Snapshot

QuantLib is a C++17 quantitative-finance library (with CI coverage through newer
language modes up to C++26). It is mature, pattern-heavy, and strongly centered
on lazy evaluation and observer-driven dependency propagation.

- **Primary language**: C++17 (`.clang-format` sets `Standard: c++17`)
- **Dependencies**: Boost headers (minimum checks in `configure.ac`; CMake
  requires newer Boost for C++20)
- **Build systems**: CMake, Autotools, and a Visual Studio solution — all three
  are exercised in CI
- **Platforms in CI**: Linux, macOS, Windows
- **Test framework**: Boost.Test (`test-suite/`)

## 2. Task Guides

This file stays short because it is loaded into context on every session.
Task-specific detail lives in [`.agents/`](.agents/README.md) and should be read
on demand:

| Read this | When |
| --- | --- |
| [`.agents/architecture.md`](.agents/architecture.md) | You need the instrument/engine lifecycle, the cash-flow hierarchy, the design patterns, or the module map. |
| [`.agents/build-and-test.md`](.agents/build-and-test.md) | You need build options, configure flags, test invocations, or the CI workflow map. |
| [`.agents/registering-new-files.md`](.agents/registering-new-files.md) | You add, rename, or remove a `.hpp`/`.cpp` file. |
| [`.agents/extending-quantlib.md`](.agents/extending-quantlib.md) | You add a new instrument, term structure, calendar, or day counter. |
| [`.agents/deprecation.md`](.agents/deprecation.md) | You deprecate, change, or remove a public API. |

## 3. Core Abstractions

- **`Instrument`** (`ql/instrument.hpp`) — lazy priced object holding a pricing
  engine and cached results.
- **`PricingEngine`** (`ql/pricingengine.hpp`) — `arguments`/`results` protocol;
  concrete engines implement `calculate()`.
- **`CashFlow` / `Leg`** (`ql/cashflow.hpp`) — cash-flow primitives; `Leg` is
  `std::vector<ext::shared_ptr<CashFlow>>`.
- **`TermStructure`** (`ql/termstructure.hpp`) — curves (yield, vol, default,
  inflation), observer-aware.
- **`Quote`** (`ql/quote.hpp`) — observable market datum, typically consumed
  through `Handle<Quote>`.

Pricing is lazy: `Instrument::NPV()` reuses cached results until an observed
dependency notifies a change, then re-runs the engine cycle. Before changing
anything that touches caching, notification, or curve construction, read
[`.agents/architecture.md`](.agents/architecture.md).

## 4. Coding Conventions

### 4.1 Formatting and Includes

Source of truth: `.clang-format`.

- 4-space indent, no tabs.
- 100-column limit.
- Namespace indentation enabled.
- Pointer/reference alignment: `T* p`, `T& x`.
- Include order: local (`"..."`) → `<ql/...>` → `<boost/...>` → standard
  headers.

### 4.2 Naming and API Style

- Types/classes: `PascalCase`
- Functions/methods: `lowerCamelCase`
- Data members: trailing underscore (`engine_`, `calculated_`)
- Macros: `QL_UPPER_CASE`
- Prefer `const` correctness and pass heavy objects by `const&`.

### 4.3 Memory and Ownership

Use the QuantLib portability aliases in `ql/shared_ptr.hpp`:

- `ext::shared_ptr<T>`
- `ext::make_shared<T>(...)`
- `ext::dynamic_pointer_cast<T>(...)`

Use `std::unique_ptr` for strict local ownership in implementations.

### 4.4 Error Handling

Use the `ql/errors.hpp` macros, not raw `throw`/`assert`:

- `QL_REQUIRE`
- `QL_ENSURE`
- `QL_FAIL`
- `QL_ASSERT`

### 4.5 Header Pattern

New headers should have:

- The standard QuantLib license block
- Doxygen `\file` and brief
- Include guard `quantlib_<name>_hpp`
- Self-contained includes

## 5. Build and Test — Quick Start

```bash
cmake --preset linux-gcc-ninja-release
cmake --build build/linux-gcc-ninja-release
./build/linux-gcc-ninja-release/test-suite/quantlib-test-suite \
    --log_level=message
```

Build options, Autotools and MSVC instructions, targeted test invocations, and
the CI workflow map are in
[`.agents/build-and-test.md`](.agents/build-and-test.md).

## 6. High-Impact Pitfalls

### 6.1 Global Settings

- `Settings::evaluationDate()` is global by default; with sessions enabled it
  becomes per-session/per-thread.
- Use `SavedSettings` in tests to avoid leakage across cases.

### 6.2 Lazy Evaluation Surprises

- Notification forwarding behavior can differ (default/per-object, compile-time
  macro influence).
- Cycle handling can be silent unless `QL_THROW_IN_CYCLES` is enabled.
- Misused `freeze()` can leave stale values.

### 6.3 Handles and Lifetimes

- `Handle<T>` defaults to `registerAsObserver=true`.
- If a handle does not own the pointee safely, observer callbacks can outlive
  valid memory.
- Relinking a `RelinkableHandle` can trigger broad recalculation cascades.

### 6.4 Dates and Conventions

- 30/360 variants differ materially.
- Actual/Actual ISMA requires schedule context for irregular periods.
- `Calendar::isEndOfMonth()` is business-day aware, not raw month-end.

### 6.5 Numerical Robustness

- Prefer robust solvers (often Brent) when derivatives are unavailable or noisy.
- Set realistic tolerances and max evaluations.
- Validate implied-vol and calibration outputs against known references.

### 6.6 Type Alias Nuance

`ql/types.hpp` aliases are macro-backed (`QL_REAL`, `QL_INTEGER`,
`QL_BIG_INTEGER` in `ql/qldefines.hpp`) and configurable; do not hard-code
assumptions beyond the defaults.

## 7. Validation Checklist

Before finishing a change:

- [ ] Build passes with no new warnings.
- [ ] Relevant tests pass.
- [ ] New behavior has tests.
- [ ] Build lists are updated where needed (`ql/*`, `test-suite/*`).
- [ ] Visual Studio project/filter files are updated for new files
      (`QuantLib.vcxproj*`, `test-suite/testsuite.vcxproj*`).
- [ ] Headers are self-contained.
- [ ] Error handling uses `QL_*` macros.
- [ ] Pointer types use `ext::shared_ptr` conventions where appropriate.
- [ ] Numerical tolerances are justified for quant outputs.

## 8. Quick Reference

- `ql/instrument.hpp` — instrument lifecycle and engine calls
- `ql/pricingengine.hpp` — engine interface and generic engine
- `ql/cashflow.hpp` — `CashFlow` and `Leg`
- `ql/cashflows/coupon.hpp`, `ql/cashflows/floatingratecoupon.hpp`,
  `ql/cashflows/couponpricer.hpp` — coupon hierarchy and pricers
- `ql/cashflows/cashflows.hpp` — leg analytics
- `ql/termstructure.hpp` — curve base
- `ql/settings.hpp` — global settings and evaluation date
- `ql/patterns/lazyobject.hpp`, `ql/patterns/observable.hpp`
- `ql/handle.hpp` — handles and relinking
- `ql/errors.hpp`, `ql/shared_ptr.hpp`, `ql/types.hpp`, `ql/qldefines.hpp`

## 9. Keep This File Lean

Update `AGENTS.md` when build/test entry points, workflow files, major
conventions, or core architecture references change. Anything that is only
needed for one kind of task belongs in [`.agents/`](.agents/README.md) instead,
so it does not consume context on every session.

Recommended cadence:

- quick check before release candidates
- deeper full pass quarterly
