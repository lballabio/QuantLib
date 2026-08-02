# qlnb — QuantLib nanobind bindings (Phase 0)

Experimental Python package that binds a small QuantLib surface with
[nanobind](https://github.com/wjakob/nanobind) and ships a wheel via
[scikit-build-core](https://github.com/scikit-build/scikit-build-core).

Phase 0 covers:

- `Date`, `Settings`
- `Quote` / `SimpleQuote`, `QuoteHandle`
- `FlatForward` / `BlackConstantVol` factories (return handles)
- `BlackScholesMertonProcess`
- `EuropeanOption` priced via `AnalyticEuropeanEngine` sugar

QuantLib is built from the parent source tree as a **static** library with
`QL_USE_STD_SHARED_PTR=ON` and `CMAKE_POSITION_INDEPENDENT_CODE=ON`.

### Design notes

nanobind does not support general multiple inheritance / base-pointer
adjustment. Types such as `FlatForward` (MI with `LazyObject`) are therefore
exposed as factories that return `Handle<...>` values rather than as Python
subclasses of the C++ hierarchy. Day counters and calendars use QuantLib's
value-semantic pimpl types (`Actual365Fixed()` / `TARGET()` return
`DayCounter` / `Calendar`).

## Build

From this directory, with a virtualenv:

```bash
python -m venv .venv
source .venv/bin/activate
pip install -U pip build pytest nanobind scikit-build-core ninja
pip install --no-build-isolation .
pytest
```

Or build a wheel:

```bash
python -m build --wheel
pip install dist/qlnb-*.whl
```

## Quick check

```python
import qlnb as ql

todays = ql.Date(15, ql.Month.May, 1998)
ql.set_evaluation_date(todays)
# see tests/test_phase0.py for the EquityOption European put example
```

## Benchmark

```bash
python benchmarks/bench_phase0.py
```

Optionally install the official `QuantLib` wheel to compare call overhead:

```bash
pip install QuantLib
python benchmarks/bench_phase0.py
```

Representative local results (Linux / CPython 3.12):

| Microbenchmark | qlnb | QuantLib (SWIG) | Speedup |
| --- | --- | --- | --- |
| `Date + 1` | ~0.08 µs | ~1.47 µs | ~18× |
| `FlatForward.discount` ×365 | ~22 µs | ~434 µs | ~19× |
