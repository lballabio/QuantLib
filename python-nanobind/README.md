# qlnb — QuantLib nanobind bindings

Experimental Python package that binds a focused QuantLib surface with
[nanobind](https://github.com/wjakob/nanobind) and ships a wheel via
[scikit-build-core](https://github.com/scikit-build/scikit-build-core).

## Status

### Phase 0
- `Date`, `Settings`, quotes/handles
- `FlatForward` / `BlackConstantVol` factories
- `BlackScholesMertonProcess`, `EuropeanOption`

### Phase 1 (market stack)
- `Period`, calendars, day counters, `Schedule`
- Deposit helpers + `PiecewiseLogLinearDiscountCurve`
- `Euribor3M` / `Euribor6M`
- `FixedRateBond` + discounting engine
- `VanillaSwap` + discounting engine
- Type stubs in `qlnb/_qlnb.pyi`

### Phase 2 (pricing coverage)
- American `VanillaOption` + Barone-Adesi-Whaley engine
- European greeks (`delta` / `gamma` / `vega`) and `implied_volatility`
- Monte Carlo European engine (`set_mc_pricing_engine`)
- `simulate_gbm_paths` → NumPy `(samples, time_steps+1)` array
- `ForwardRateAgreement` (reuses curve / `Euribor` bindings)
- New translation unit: `src/bind_pricing.cpp`

### Phase 3 (productization)
- cibuildwheel workflow (manylinux x86_64 Stable ABI / cp312)
- Migration guide vs official SWIG bindings
- Benchmark + NPV drift CI job
- Packaging notes for monorepo wheel builds

### Phase 4 (stretch)
- `qlnb.compat` — best-effort SWIG-style aliases (`ql.May`, `Option.Put`,
  camelCase methods, `evaluationDate`); not full SWIG parity
- Experimental pricing: `BarrierOption` + `AnalyticBarrierEngine`,
  `make_cap` / `make_floor` + Black cap/floor engine
- Free-threading notes (`docs/free-threading.md`) and optional
  `QLNB_THREAD_SAFE_OBSERVER` CMake passthrough

### Phase 5 (rates options + ergonomics)
- European `Swaption` + Black swaption engine (`src/bind_rates_options.cpp`)
- `ZeroCouponBond` + discounting engine (same pattern as `FixedRateBond`)
- `make_vanilla_swap` helper for MakeVanillaSwap-style construction
- NumPy helpers: `discount_times` / `discount_dates` on curve handles
- Bootstrap helpers: `FraRateHelper`, `SwapRateHelper`
- Calendars: `Japan()`, `Germany(...)`
- Expanded `qlnb.compat` aliases (`Settlement.*`, bond/swaption camelCase)

### Phase 6 (floating bonds, tree/FD, overnight indexes)
- `FloatingRateBond` + discounting engine with `BlackIborCouponPricer` setup
- Cox–Ross–Rubinstein binomial and FD Black–Scholes engines on `VanillaOption`
  (`set_binomial_pricing_engine`, `set_fd_pricing_engine`)
- Overnight indexes: `Sofr`, `Estr`, `Eonia` (`OvernightIndex`)
- `make_ois` → `OvernightIndexedSwap` + discounting engine
- `qlnb.compat` aliases for floating bonds / tree-FD / OIS

QuantLib is built from the parent source tree as a **static** library with
`QL_USE_STD_SHARED_PTR=ON` and `CMAKE_POSITION_INDEPENDENT_CODE=ON`.

### Design notes

nanobind does not support general multiple inheritance / base-pointer
adjustment. MI-heavy types (`FlatForward`, bonds/swaps, rate helpers, FRA)
are exposed as factories or standalone wrappers rather than mirroring the
full C++ hierarchy. Day counters and calendars use QuantLib's value-semantic
pimpl types. Option engines are attached via lambdas on concrete wrappers
(no Instrument/OneAssetOption MI hierarchy in Python).

## Docs

- [SWIG → qlnb migration guide](docs/migration.md)
- [Packaging / wheel build notes](docs/packaging.md)
- [Free-threading readiness](docs/free-threading.md)

Compatibility shim (optional):

```python
import qlnb.compat as ql   # SWIG-flavored aliases; prefer native qlnb for new code
d = ql.Date(15, ql.May, 1998)
ql.Settings.instance().evaluationDate = d
```

## Build

Wheels must be built from a **full QuantLib checkout** (the parent tree is
compiled into the extension). An sdist of `python-nanobind/` alone is not
sufficient — see [docs/packaging.md](docs/packaging.md).

```bash
python -m venv .venv
source .venv/bin/activate
pip install -U pip pytest numpy nanobind scikit-build-core ninja
pip install --no-build-isolation .
pytest
```

Optional local wheel:

```bash
pip install build
python -m build --wheel
```

## Benchmark / NPV drift

```bash
python benchmarks/bench_phase0.py
python scripts/check_npv_drift.py --abs-tol 1e-8
pip install QuantLib   # optional SWIG comparison
python benchmarks/bench_phase0.py
```

CI builds manylinux wheels and uploads benchmark artifacts via
`.github/workflows/qlnb-wheels.yml`.
