# QuantLib Python Drop-In Replacement

> **This is a pure Python reimplementation of the QuantLib C++ library.**
> The goal is full API compatibility: Python code written against this library should work identically to code written against the original QuantLib C++ library (via its Python bindings), with no C++ compilation or native extensions required.

## What This Project Is

This repository contains a **pure Python drop-in replacement** for QuantLib. It mirrors the QuantLib API so that existing QuantLib Python users can switch to this library by changing only their import statements — all function signatures, class names, and behavior remain the same.

**Target users**: Python developers who want QuantLib functionality without installing C++ bindings (`QuantLib-SWIG` / `QuantLib-Python`).

## What This Project Is NOT

- This is not a wrapper around the C++ QuantLib library.
- This is not a simplified or partial reimplementation — the goal is full feature parity.
- This is not a new quant finance API — it deliberately copies QuantLib's existing API design.

## Scope

The replacement should maintain API compatibility with the existing QuantLib library where practical, allowing users to migrate or use Python code with minimal changes.

---

## Development Workflow

### GitHub Flow

All work follows **GitHub Flow**. Never commit directly to `master`.

1. **Branch** — create a feature branch from `master` (`task/<issue-number>-<short-desc>`)
2. **Commit** — make changes, commit to the feature branch
3. **Push** — push the branch to `origin`
4. **Pull Request** — open a PR against `master`, linking the GitHub Issue
5. **CI** — GitHub Actions runs lint, typecheck, and tests. All must pass.
6. **Review** — PR is reviewed (by agent or human)
7. **Merge** — squash-merge into `master`. Delete the branch.

### Branch Naming

- Feature branches: `task/<issue-number>-<short-description>`
- Example: `task/2-foundational-enums`

### GitHub Issues as Source of Truth

All planning lives in GitHub Issues, not local files:
- **Epics** (`type/epic`): Define "what" — product briefs, acceptance criteria
- **Stories** (`type/story`): Define "how" — technical approach, files to create/modify
- **Tasks** (`type/task`): Single-session TDD units

Issues use `status/*` labels for workflow state and `<!-- AGENT-META -->` blocks for dependency tracking.

### TDD via C++ Test Transpilation

Do NOT write tests from scratch. Auto-transpile existing C++ Boost.Test files (`upstream/test-suite/`) to equivalent pytest files:
1. **TRANSPILE** — read C++ test file, generate pytest equivalent
2. **RED** — run tests, all fail (implementation doesn't exist)
3. **GREEN** — write minimum code to make tests pass
4. **REFACTOR** — clean up if needed

### Tracer Bullet Development

Work narrow and deep. Complete one area end-to-end before starting the next. Never go wide.

### Pre-commit Hooks

`ruff` linting and formatting run on every commit via pre-commit hooks.

### CI/CD

GitHub Actions workflow (`.github/workflows/python-tests.yml`) runs on all PRs:
- Python 3.10, 3.11, 3.12 matrix
- `make lint` → `make typecheck` → `make test-cov`
- All checks must pass before merge
- PRs auto-merge when CI passes (auto-merge enabled)
- Dependent issues auto-advance to `status/ready` when deps close

### Model Selection

Use the cheapest model that can reliably complete each task. See `.claude/model-selection.md` for the full guide.
- **haiku**: GitHub ops, label changes, git commands, CI monitoring, simple enums
- **sonnet**: Test transpilation, pattern-following implementations, new calendars/day counters
- **opus**: Complex algorithms, architecture, debugging, new patterns

### Module Context

Per-module context files at `.claude/memory/modules/<module>.md` provide architecture snapshots. Read before starting work on a module to avoid re-analyzing code.

---

## Repository Structure

```
├── quantlib/          # Python source — the drop-in replacement
├── tests/             # pytest test suite
├── upstream/          # Original QuantLib C++ source (read-only reference)
│   ├── ql/            # C++ headers/source to translate from
│   └── test-suite/    # C++ Boost.Test files to transpile into pytest
├── .github/workflows/ # CI (python-tests.yml only)
├── pyproject.toml     # Project metadata and dependencies
├── Makefile           # Dev commands: make test, make lint, make typecheck
└── CLAUDE.md          # This file
```

- **`upstream/`** contains the original C++ QuantLib code. Use it as a reference for API signatures and as the source for test transpilation. Never modify files in `upstream/`.
- **`quantlib/`** is the Python package. Mirror the C++ namespace structure (e.g., `ql/time/` → `quantlib/time/`).
- **`tests/`** contains pytest files transpiled from `upstream/test-suite/`.

---

## QuantLib Library Overview

QuantLib is a comprehensive, free/open-source C++ framework for quantitative finance: modeling, trading, and risk management. It is BSD-licensed and widely used in industry and academia.

### Instruments (`upstream/ql/instruments/`)

- **Options**: European, Asian, Barrier, Double Barrier, Soft Barrier, Lookback, Basket, Clique, Compound, Chooser (simple/complex), Margrabe, Quanto, Forward vanilla, Holder/Writer extensible, Part-time barrier
- **Bonds**: Fixed rate, Floating rate, Zero coupon, Amortizing (fixed/floating/CMS), Convertible, CPI-linked (inflation)
- **Swaps**: Vanilla, Float-float, OIS (overnight-indexed), BMA, CPI, Year-on-year inflation, FX forward, Asset swap, Multiple resets, Zero coupon
- **Interest Rate Derivatives**: Caps/Floors, Swaptions (standard, non-standard, float-float), FRAs, Overnight index futures, Perpetual futures
- **Credit**: Credit Default Swaps (CDS)
- **Equity**: Total return swap, Variance swap, Vanilla storage/swing options

### Pricing Engines (`upstream/ql/pricingengines/`)

- **Analytic**: Black-Scholes, Bachelier, Black-Delta, Black formula calculators
- **Monte Carlo**: Generic MC simulation engine, Longstaff-Schwartz (American options)
- **Finite Differences**: PDE-based engines
- **Specialized engines per asset class**: Asian, Barrier, Basket, Capfloor, Cliquet, Credit, Exotic, Forward, Inflation, Lookback, Quanto, Swap, Swaption, Vanilla

### Models (`upstream/ql/models/`)

- **Short Rate (1-factor)**: Vasicek, Hull-White, Cox-Ingersoll-Ross (CIR), Extended CIR, Black-Karasinski, Gaussian1d, GSR, Markov Functional
- **Short Rate (2-factor)**: G2
- **Equity/Stochastic Vol**: Heston, Bates, GJR-GARCH, Heston SLV (FDM/MC)

### Term Structures (`upstream/ql/termstructures/`)

- **Yield curves**: Piecewise (bootstrapped), flat forward, discount curve, zero curve, fitted bond curve (Nelson-Siegel, Svensson, etc.)
- **Volatility**: SABR, ZABR, ABCD, smile sections, cap/floor vol, swaption vol, equity/FX vol surfaces
- **Default (credit)**: Default probability term structures
- **Inflation**: CPI and year-on-year inflation term structures

### Math & Numerical Methods (`upstream/ql/math/`, `upstream/ql/methods/`)

- **Numerical methods**: Monte Carlo, Finite Differences, Lattices (binomial/trinomial trees)
- **Optimization**: 1D solvers, multi-dimensional optimization
- **Interpolation**: Linear, cubic spline, bilinear, many others
- **Statistics**: Distributions, copulas, autocovariance
- **Random numbers**: Sobol, Mersenne Twister, Box-Muller, and many more
- **Integration, FFT, ODE solvers**

### Time/Calendar (`upstream/ql/time/`)

- Business day calendars for major markets worldwide
- Day count conventions (Actual/360, Actual/365, 30/360, etc.)
- Schedule generation, tenor handling

### Real-World Usage

- **Investment banks**: Derivatives pricing desks, fair value calculation
- **Risk management**: VaR, Greeks, stress testing, Basel IV compliance
- **Asset managers**: Bond portfolio management, yield curve analysis
- **Hedge funds**: Growing adoption for systematic and quantitative strategies
- **Fintech / robo-advisors**: Core pricing engine
- **Algorithmic trading platforms**: Embedded analytics
- **Academia**: Research and model prototyping
