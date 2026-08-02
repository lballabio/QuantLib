#!/usr/bin/env python3
"""Microbenchmarks for phase-0 qlnb hot paths.

Compares against the official QuantLib SWIG wheel when installed.
"""

from __future__ import annotations

import time


def _bench(label: str, fn, loops: int) -> float:
    fn()
    start = time.perf_counter()
    for _ in range(loops):
        fn()
    elapsed = time.perf_counter() - start
    per_call_us = (elapsed / loops) * 1e6
    print(f"{label:40s} {loops:8d} calls  {per_call_us:10.3f} us/call")
    return per_call_us


def bench_qlnb():
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

    print("=== qlnb (nanobind) ===")
    _bench("Date + 1 day", date_add, 200_000)
    _bench("FlatForward.discount 365x", discount_loop, 2_000)


def bench_swig():
    try:
        import QuantLib as ql
    except ImportError:
        print("=== official QuantLib (SWIG) ===")
        print("skipped (pip install QuantLib to compare)")
        return

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

    print("=== official QuantLib (SWIG) ===")
    _bench("Date + 1 day", date_add, 200_000)
    _bench("FlatForward.discount 365x", discount_loop, 2_000)


if __name__ == "__main__":
    bench_qlnb()
    print()
    bench_swig()
