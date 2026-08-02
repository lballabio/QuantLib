#!/usr/bin/env python3
"""Fail if the EquityOption European put NPV drifts from the golden value.

Used by CI (and locally) to catch accidental pricing regressions in qlnb.
Tolerance defaults to 1e-8 absolute (stricter than many market tests; the
unit tests use ~1e-10).
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


# EquityOption example (Black-Scholes European put) — same golden as tests.
GOLDEN_NPV = 3.844307791471182
DEFAULT_ABS_TOL = 1e-8


def price_european_put() -> float:
    import qlnb as ql

    calendar = ql.TARGET()
    todays_date = ql.Date(15, ql.Month.May, 1998)
    settlement_date = ql.Date(17, ql.Month.May, 1998)
    ql.set_evaluation_date(todays_date)

    underlying = 36.0
    strike = 40.0
    dividend_yield = 0.00
    risk_free_rate = 0.06
    volatility = 0.20
    maturity = ql.Date(17, ql.Month.May, 1999)
    day_counter = ql.Actual365Fixed()

    underlying_h = ql.make_quote_handle(underlying)
    flat_ts = ql.FlatForward(settlement_date, risk_free_rate, day_counter)
    flat_dividend_ts = ql.FlatForward(settlement_date, dividend_yield, day_counter)
    flat_vol_ts = ql.BlackConstantVol(
        settlement_date, calendar, volatility, day_counter
    )

    process = ql.BlackScholesMertonProcess(
        underlying_h, flat_dividend_ts, flat_ts, flat_vol_ts
    )
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Put, strike)
    exercise = ql.EuropeanExercise(maturity)
    option = ql.EuropeanOption(payoff, exercise)
    option.set_pricing_engine(ql.AnalyticEuropeanEngine(process))
    return float(option.NPV())


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--abs-tol",
        type=float,
        default=DEFAULT_ABS_TOL,
        help=f"absolute NPV tolerance (default {DEFAULT_ABS_TOL})",
    )
    parser.add_argument(
        "--json-out",
        type=Path,
        default=None,
        help="optional path to write a JSON summary",
    )
    args = parser.parse_args(argv)

    npv = price_european_put()
    drift = abs(npv - GOLDEN_NPV)
    ok = drift <= args.abs_tol

    summary = {
        "case": "EquityOption European put (BS)",
        "qlnb_npv": npv,
        "golden_npv": GOLDEN_NPV,
        "abs_drift": drift,
        "abs_tol": args.abs_tol,
        "ok": ok,
    }

    status = "OK" if ok else "FAIL"
    print(
        f"[{status}] EquityOption European put NPV={npv:.12f} "
        f"golden={GOLDEN_NPV:.12f} |drift|={drift:.3e} tol={args.abs_tol:.3e}"
    )

    if args.json_out is not None:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
        print(f"wrote {args.json_out}")

    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
