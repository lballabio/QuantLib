#!/usr/bin/env python3
"""Microbenchmarks for qlnb hot paths (phase 0–2).

Compares against the official QuantLib SWIG wheel when installed.
Also prints the EquityOption European put NPV for CI artifact capture.
"""

from __future__ import annotations

import json
import os
import time
from pathlib import Path


GOLDEN_NPV = 3.844307791471182


def _bench(label: str, fn, loops: int) -> float:
    fn()
    start = time.perf_counter()
    for _ in range(loops):
        fn()
    elapsed = time.perf_counter() - start
    per_call_us = (elapsed / loops) * 1e6
    print(f"{label:40s} {loops:8d} calls  {per_call_us:10.3f} us/call")
    return per_call_us


def _price_qlnb_european_put():
    import qlnb as ql

    calendar = ql.TARGET()
    todays_date = ql.Date(15, ql.Month.May, 1998)
    settlement_date = ql.Date(17, ql.Month.May, 1998)
    ql.set_evaluation_date(todays_date)
    day_counter = ql.Actual365Fixed()

    process = ql.BlackScholesMertonProcess(
        ql.make_quote_handle(36.0),
        ql.FlatForward(settlement_date, 0.00, day_counter),
        ql.FlatForward(settlement_date, 0.06, day_counter),
        ql.BlackConstantVol(settlement_date, calendar, 0.20, day_counter),
    )
    option = ql.EuropeanOption(
        ql.PlainVanillaPayoff(ql.OptionType.Put, 40.0),
        ql.EuropeanExercise(ql.Date(17, ql.Month.May, 1999)),
    )
    option.set_pricing_engine(process)
    return option.NPV()


def bench_qlnb() -> dict[str, float]:
    import qlnb as ql

    d0 = ql.Date(15, ql.Month.May, 1998)
    ref = ql.Date(17, ql.Month.May, 1998)
    ql.set_evaluation_date(d0)
    dc = ql.Actual365Fixed()
    curve = ql.FlatForward(ref, 0.06, dc)
    dates = [ref + i for i in range(1, 366)]

    def date_add():
        return d0 + 1

    def discount_loop():
        total = 0.0
        for d in dates:
            total += curve.discount(d)
        return total

    def price_once():
        return _price_qlnb_european_put()

    print("=== qlnb (nanobind) ===")
    results = {
        "date_add_us": _bench("Date + 1 day", date_add, 200_000),
        "discount_365x_us": _bench("FlatForward.discount 365x", discount_loop, 2_000),
        "european_put_us": _bench("European put NPV", price_once, 2_000),
    }
    npv = _price_qlnb_european_put()
    print(f"{'European put NPV':40s} {npv:18.12f}  (golden {GOLDEN_NPV:.12f})")
    results["european_put_npv"] = float(npv)
    results["golden_npv"] = GOLDEN_NPV
    results["npv_abs_drift"] = abs(float(npv) - GOLDEN_NPV)
    return results


def bench_swig() -> dict[str, float] | None:
    try:
        import QuantLib as ql
    except ImportError:
        print("=== official QuantLib (SWIG) ===")
        print("skipped (pip install QuantLib to compare)")
        return None

    d0 = ql.Date(15, ql.May, 1998)
    ref = ql.Date(17, ql.May, 1998)
    ql.Settings.instance().evaluationDate = d0
    dc = ql.Actual365Fixed()
    curve = ql.FlatForward(ref, 0.06, dc)
    dates = [ref + i for i in range(1, 366)]

    def date_add():
        return d0 + 1

    def discount_loop():
        total = 0.0
        for d in dates:
            total += curve.discount(d)
        return total

    def price_once():
        calendar = ql.TARGET()
        settlement = ql.Date(17, ql.May, 1998)
        process = ql.BlackScholesMertonProcess(
            ql.QuoteHandle(ql.SimpleQuote(36.0)),
            ql.YieldTermStructureHandle(ql.FlatForward(settlement, 0.00, dc)),
            ql.YieldTermStructureHandle(ql.FlatForward(settlement, 0.06, dc)),
            ql.BlackVolTermStructureHandle(
                ql.BlackConstantVol(settlement, calendar, 0.20, dc)
            ),
        )
        option = ql.EuropeanOption(
            ql.PlainVanillaPayoff(ql.Option.Put, 40.0),
            ql.EuropeanExercise(ql.Date(17, ql.May, 1999)),
        )
        option.setPricingEngine(ql.AnalyticEuropeanEngine(process))
        return option.NPV()

    print("=== official QuantLib (SWIG) ===")
    results = {
        "date_add_us": _bench("Date + 1 day", date_add, 200_000),
        "discount_365x_us": _bench("FlatForward.discount 365x", discount_loop, 2_000),
        "european_put_us": _bench("European put NPV", price_once, 2_000),
    }
    results["european_put_npv"] = float(price_once())
    print(
        f"{'European put NPV':40s} {results['european_put_npv']:18.12f}"
    )
    return results


def main() -> None:
    qlnb_results = bench_qlnb()
    print()
    swig_results = bench_swig()

    out_dir = Path(__file__).resolve().parent / "out"
    artifact_dir = Path(os.environ["QLNB_BENCH_OUT"]) if "QLNB_BENCH_OUT" in os.environ else out_dir
    artifact_dir.mkdir(parents=True, exist_ok=True)
    payload = {"qlnb": qlnb_results, "swig": swig_results}
    out_path = artifact_dir / "bench_phase0.json"
    out_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print()
    print(f"wrote {out_path}")


if __name__ == "__main__":
    main()
