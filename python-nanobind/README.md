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

QuantLib is built from the parent source tree as a **static** library with
`QL_USE_STD_SHARED_PTR=ON` and `CMAKE_POSITION_INDEPENDENT_CODE=ON`.

### Design notes

nanobind does not support general multiple inheritance / base-pointer
adjustment. MI-heavy types (`FlatForward`, bonds/swaps, rate helpers) are
exposed as factories or standalone wrappers rather than mirroring the full
C++ hierarchy. Day counters and calendars use QuantLib's value-semantic
pimpl types.

## Build

```bash
python -m venv .venv
source .venv/bin/activate
pip install -U pip pytest nanobind scikit-build-core ninja
pip install --no-build-isolation .
pytest
```

## Benchmark

```bash
python benchmarks/bench_phase0.py
pip install QuantLib   # optional SWIG comparison
python benchmarks/bench_phase0.py
```
