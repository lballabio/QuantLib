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

QuantLib is built from the parent source tree as a **static** library with
`QL_USE_STD_SHARED_PTR=ON` and `CMAKE_POSITION_INDEPENDENT_CODE=ON`.

### Design notes

nanobind does not support general multiple inheritance / base-pointer
adjustment. MI-heavy types (`FlatForward`, bonds/swaps, rate helpers, FRA)
are exposed as factories or standalone wrappers rather than mirroring the
full C++ hierarchy. Day counters and calendars use QuantLib's value-semantic
pimpl types. Option engines are attached via lambdas on concrete wrappers
(no Instrument/OneAssetOption MI hierarchy in Python).

## Build

```bash
python -m venv .venv
source .venv/bin/activate
pip install -U pip pytest numpy nanobind scikit-build-core ninja
pip install --no-build-isolation .
pytest
```

## Benchmark

```bash
python benchmarks/bench_phase0.py
pip install QuantLib   # optional SWIG comparison
python benchmarks/bench_phase0.py
```
