# Migrating from QuantLib-SWIG to qlnb

`qlnb` is an experimental nanobind binding surface for QuantLib. It aims for
familiar APIs where practical, but **does not** mirror the full SWIG hierarchy.

Import style is intentionally close:

```python
import qlnb as ql   # instead of: import QuantLib as ql
```

## Design differences (read first)

nanobind does not support general multiple inheritance / base-pointer
adjustment. Types that are MI-heavy in C++ (`FlatForward`, bonds, swaps, rate
helpers, FRAs, option engines) are exposed as **factories** or **concrete
wrappers** rather than as full class hierarchies.

Consequences:

- `FlatForward(...)` returns a `YieldTermStructureHandle`, not a curve object.
- `BlackConstantVol(...)` returns a vol handle suitable for processes.
- Bond/swap/option engines are attached via `set_pricing_engine(...)` helpers
  that take handles or processes — you do not construct `DiscountingBondEngine`
  / `AnalyticEuropeanEngine` objects yourself in most paths (factory aliases
  exist for options).
- Day counters and calendars are value-semantic pimpl types (construct via
  factory functions: `ql.Actual365Fixed()`, `ql.TARGET()`, …).

## Dates and Settings

| SWIG (`QuantLib`) | qlnb |
| --- | --- |
| `ql.Date(15, ql.May, 1998)` | `ql.Date(15, ql.Month.May, 1998)` |
| `ql.Settings.instance().evaluationDate = d` | `ql.set_evaluation_date(d)` or `ql.Settings.instance().evaluation_date = d` |
| `ql.Settings.instance().evaluationDate` | `ql.get_evaluation_date()` / `.evaluation_date` |
| camelCase members (`dayOfMonth`) | snake_case (`day_of_month`) |

```python
import qlnb as ql

d = ql.Date(15, ql.Month.May, 1998)
ql.set_evaluation_date(d)
assert ql.get_evaluation_date() == d
```

Months live under `ql.Month.*`. Option types under `ql.OptionType.*` (not
`ql.Option.Put`).

## Quotes, handles, factories

```python
# SWIG
q = ql.SimpleQuote(36.0)
h = ql.QuoteHandle(q)

# qlnb — same, plus a convenience factory
q = ql.SimpleQuote(36.0)
h = ql.QuoteHandle(q)
h2 = ql.make_quote_handle(36.0)
```

Relinkable quote handles are available as `ql.RelinkableQuoteHandle`.

## FlatForward returns a handle

```python
# SWIG
curve = ql.FlatForward(ref, 0.06, dc)
ts = ql.YieldTermStructureHandle(curve)

# qlnb — FlatForward already returns YieldTermStructureHandle
ts = ql.FlatForward(ref, 0.06, dc)
print(ts.discount(maturity))
```

The same pattern applies to `PiecewiseLogLinearDiscountCurve(...)` and
`BlackConstantVol(...)` (vol handle).

## Schedule, calendars, day counters

```python
cal = ql.TARGET()
dc = ql.Actual365Fixed()
tenor = ql.Period(6, ql.TimeUnit.Months)
sched = ql.Schedule(
    start,
    end,
    tenor,
    cal,
    ql.BusinessDayConvention.ModifiedFollowing,
    ql.BusinessDayConvention.ModifiedFollowing,
    ql.DateGeneration.Forward,
    False,
)
dates = sched.dates()
```

Enums are nested (`ql.BusinessDayConvention.ModifiedFollowing`,
`ql.DateGeneration.Forward`, `ql.Frequency.Semiannual`).

## Bonds and swaps

Engines take a discount curve handle directly:

```python
bond = ql.FixedRateBond(
    settlement_days,
    face,
    schedule,
    [0.05],
    ql.ActualActual(ql.ActualActualConvention.ISDA),
)
bond.set_pricing_engine(discount_curve)  # YieldTermStructureHandle
npv = bond.NPV()

swap = ql.VanillaSwap(
    ql.SwapType.Payer,
    notional,
    fixed_schedule,
    fixed_rate,
    fixed_dc,
    float_schedule,
    ql.Euribor6M(forecast_curve),
    0.0,
    float_dc,
)
swap.set_pricing_engine(discount_curve)
print(swap.fair_rate(), swap.NPV())
```

Deposit helpers and piecewise curves:

```python
helpers = [
    ql.DepositRateHelper(
        0.01,
        ql.Period(3, ql.TimeUnit.Months),
        2,
        cal,
        ql.BusinessDayConvention.ModifiedFollowing,
        True,
        ql.Actual360(),
    ),
]
curve = ql.PiecewiseLogLinearDiscountCurve(ref, helpers, ql.Actual365Fixed())
```

## Options and engines

```python
process = ql.BlackScholesMertonProcess(
    ql.make_quote_handle(spot),
    dividend_ts,   # YieldTermStructureHandle
    risk_free_ts,  # YieldTermStructureHandle
    ql.BlackConstantVol(ref, cal, vol, dc),
)

payoff = ql.PlainVanillaPayoff(ql.OptionType.Put, strike)
exercise = ql.EuropeanExercise(maturity)
option = ql.EuropeanOption(payoff, exercise)

# Preferred: pass the process; engine is constructed inside the binding
option.set_pricing_engine(process)

# Or use the AnalyticEuropeanEngine factory alias (returns the process token)
option.set_pricing_engine(ql.AnalyticEuropeanEngine(process))

npv = option.NPV()
delta = option.delta()
iv = option.implied_volatility(npv, process)
```

American options use `ql.VanillaOption` + `ql.AmericanExercise` and
`set_pricing_engine` / `BaroneAdesiWhaleyEngine`.

Monte Carlo European:

```python
option.set_mc_pricing_engine(
    process,
    time_steps=84,
    required_samples=50_000,
    seed=42,
)
```

Forward rate agreements:

```python
fra = ql.ForwardRateAgreement(
    ql.Euribor3M(forecast),
    value_date,
    ql.Position.Long,
    strike,
    notional,
    discount,
)
print(fra.NPV(), float(fra.forward_rate()))
```

## NumPy path helper

```python
import numpy as np

paths = ql.simulate_gbm_paths(
    process,
    length=1.0,
    time_steps=84,
    samples=1_000,
    seed=7,
)
# shape: (samples, time_steps + 1)
assert isinstance(paths, np.ndarray)
```

Requires NumPy at import/use time (`pip install qlnb[numpy]` or the `test` extra).

## Quick SWIG → qlnb cheat sheet

| Topic | SWIG | qlnb |
| --- | --- | --- |
| Import | `import QuantLib as ql` | `import qlnb as ql` |
| Month enum | `ql.May` | `ql.Month.May` |
| Put/Call | `ql.Option.Put` | `ql.OptionType.Put` |
| Evaluation date | `Settings.instance().evaluationDate` | `set_evaluation_date` / `.evaluation_date` |
| Flat forward | curve object → wrap in handle | factory returns handle |
| Engines | construct engine, `setPricingEngine` | `set_pricing_engine(handle_or_process)` |
| Naming | camelCase | snake_case |
| Coverage | broad SWIG surface | focused phase 0–12 surface |

## Compatibility shim (`qlnb.compat`)

For scripts that expect SWIG-ish names, import the optional shim:

```python
import qlnb.compat as ql

d = ql.Date(15, ql.May, 1998)                 # module-level month
ql.Settings.instance().evaluationDate = d     # camelCase property
payoff = ql.PlainVanillaPayoff(ql.Option.Put, 40.0)
option = ql.EuropeanOption(payoff, ql.EuropeanExercise(d + 365))
option.setPricingEngine(process)              # camelCase method alias
bond.cleanPrice()                             # alias of clean_price()
```

**This is not full SWIG parity.** Prefer the native snake_case qlnb API for new
code. The shim only covers common renames documented above (months, Option
namespace, Settings, and camelCase aliases on types already bound by qlnb).

## Phase-4 instruments

Barrier options and caps/floors follow the same factory / concrete-wrapper
pattern (no Instrument MI hierarchy in Python):

```python
barrier = ql.BarrierOption(
    ql.BarrierType.DownIn,
    90.0,
    0.0,
    ql.PlainVanillaPayoff(ql.OptionType.Call, 100.0),
    ql.EuropeanExercise(maturity),
)
barrier.set_pricing_engine(process)  # AnalyticBarrierEngine

cap = ql.make_cap(ql.Period(5, ql.TimeUnit.Years), ql.Euribor6M(curve), 0.07)
cap.set_pricing_engine(curve, volatility=0.20)
```

## Phase-5 rates options and curve helpers

European swaptions and zero-coupon bonds use the same standalone-wrapper
pattern. Engines are attached via `set_pricing_engine` helpers (no MI engine
hierarchy in Python):

```python
swap = ql.make_vanilla_swap(
    ql.Period(10, ql.TimeUnit.Years),
    ql.Euribor6M(curve),
    0.06,
    effective_date,
)
swaption = ql.Swaption(swap, ql.EuropeanExercise(exercise_date))
swaption.set_pricing_engine(curve, volatility=0.20)

zcb = ql.ZeroCouponBond(2, ql.TARGET(), 100.0, maturity)
zcb.set_pricing_engine(curve)
```

Vectorized discount factors (requires NumPy):

```python
import numpy as np

dfs = ql.discount_times(curve, np.array([0.5, 1.0, 2.0]))
dfs2 = ql.discount_dates(curve, [d1, d2, d3])
```

`FraRateHelper` / `SwapRateHelper` extend the deposit-only bootstrap surface.
`qlnb.compat` adds `Settlement.Physical`, `VanillaSwap.Payer`, and camelCase
aliases on `Swaption` / `ZeroCouponBond`.

## Phase-6 floating bonds, tree/FD, overnight indexes

Floating-rate bonds attach a discounting engine **and** a Black Ibor coupon
pricer inside `set_pricing_engine`:

```python
bond = ql.FloatingRateBond(
    2, 100.0, schedule, ql.Euribor6M(forecast), ql.ActualActual(ql.ActualActualConvention.ISDA)
)
bond.set_pricing_engine(discount)  # DiscountingBondEngine + BlackIborCouponPricer
```

American (or European) vanilla options can use tree / FD engines:

```python
opt = ql.VanillaOption(payoff, ql.AmericanExercise(today, maturity))
opt.set_binomial_pricing_engine(process, steps=801)   # Cox–Ross–Rubinstein
opt.set_fd_pricing_engine(process, t_grid=100, x_grid=100)
```

Overnight indexes and a thin MakeOIS helper. `OvernightIndex` is a standalone
opaque wrapper (not a Python subclass of `IborIndex` — Index MI):

```python
sofr = ql.Sofr(curve)
estr = ql.Estr(curve)
ois = ql.make_ois(ql.Period(2, ql.TimeUnit.Years), sofr, 0.03)
ois.set_pricing_engine(curve)
print(ois.fair_rate(), ois.NPV())
```

`FloatingRateBond(..., fixing_days=0)` means “use the index default”
(`Null<Natural>` in C++). Pass a positive value to override.

`qlnb.compat` adds camelCase aliases (`setBinomialPricingEngine`,
`FloatingRateBond.cleanPrice`, `makeOIS`).

## Phase-7 CDS, Bermudan tree swaption, FD mesher

Credit default swaps use a flat hazard-rate factory that returns a
`DefaultProbabilityTermStructureHandle` (same handle-factory pattern as
`FlatForward`):

```python
prob = ql.FlatHazardRate(today, 0.01234, ql.Actual360())
# or: ql.FlatHazardRate(0, calendar, 0.01234, ql.Actual360())
cds = ql.CreditDefaultSwap(
    ql.ProtectionSide.Seller,  # compat: ql.Protection.Seller
    notional,
    spread,
    schedule,
    ql.BusinessDayConvention.ModifiedFollowing,
    ql.Actual360(),
)
cds.set_pricing_engine(prob, recovery_rate=0.4, discount_curve=curve)
print(cds.NPV(), cds.fair_spread())
```

Bermudan swaptions attach a Hull–White tree engine (no Gaussian1d/LGM stack):

```python
model = ql.HullWhite(curve, a=0.05, sigma=0.006)
berm = ql.Swaption(swap, ql.BermudanExercise(exercise_dates))
berm.set_tree_pricing_engine(model, time_steps=50)

euro = ql.Swaption(swap, ql.EuropeanExercise(exercise_dates[0]))
euro.set_jamshidian_pricing_engine(model)  # same HW model, European
```

FD mesher locations as NumPy (requires NumPy):

```python
import numpy as np

x = ql.uniform_1d_mesher_locations(0.0, 100.0, 51)
ln_s = ql.fdm_black_scholes_mesher_locations(101, process, maturity=1.0, strike=100.0)
```

`qlnb.compat` adds `Protection.Buyer` / `Protection.Seller`, camelCase CDS /
swaption tree aliases, and `uniform1dMesherLocations`.

## Phase-8 ISDA CDS, GSR / Gaussian1d, FD value grid

ISDA Standard Model engine (same handle args as mid-point, plus numerical
flags):

```python
cds.set_isda_pricing_engine(
    prob,
    recovery_rate=0.4,
    discount_curve=curve,
    numerical_fix=ql.IsdaCdsNumericalFix.Taylor,
    accrual_bias=ql.IsdaCdsAccrualBias.HalfDayBias,
    forwards_in_coupon_period=ql.IsdaCdsForwardsInCouponPeriod.Piecewise,
)
```

Piecewise hazard rates via BackwardFlat interpolation:

```python
haz = ql.InterpolatedHazardRateCurve(
    [today, today + ql.Period(5, ql.TimeUnit.Years)],
    [0.01, 0.015],
    ql.Actual365Fixed(),
    ql.TARGET(),
)
```

GSR / Gaussian1d European swaption (constant reversion / vol):

```python
model = ql.Gsr(curve, [], [0.01], 0.01, T=50.0)
swaption.set_gaussian1d_pricing_engine(model)
```

FD value grid export (European Black–Scholes; columns `[spot, value]`):

```python
grid = ql.fdm_black_scholes_values(
    process, strike=100.0, maturity=1.0, option_type=ql.OptionType.Call,
    t_grid=50, x_grid=51,
)
# grid.shape == (51, 2)
```

## Phase-9 CDS bootstrap, Asians, FD Hull–White swaption

Hazard-rate bootstrap from spread CDS quotes (same pattern as
`DepositRateHelper` + piecewise yield curves):

```python
helpers = [
    ql.SpreadCdsHelper(
        0.005, ql.Period(1, ql.TimeUnit.Years), 1, cal,
        ql.Frequency.Quarterly, ql.BusinessDayConvention.Following,
        ql.DateGeneration.TwentiethIMM, ql.Thirty360(...), 0.4, discount,
    ),
    # ...
]
hazard = ql.PiecewiseHazardRateCurve(today, helpers, ql.Thirty360(...))
# For fair-spread round-trip vs quotes, set:
ql.Settings.instance().include_todays_cash_flows = True
```

Geometric Asian options (analytic engines only in this phase):

```python
cont = ql.ContinuousAveragingAsianOption(
    ql.AverageType.Geometric, payoff, exercise
)
cont.set_pricing_engine(process)  # continuous geometric average-price

disc = ql.DiscreteAveragingAsianOption(
    ql.AverageType.Geometric, 1.0, 0, fixing_dates, payoff, exercise
)
disc.set_pricing_engine(process)  # discrete geometric average-price
```

FD Hull–White Bermudan swaption:

```python
berm.set_fd_hullwhite_pricing_engine(hw_model, t_grid=100, x_grid=100)
```

## Phase-10 CMS / SwapIndex / Hagan pricers

```python
swap_index = ql.EuriborSwapIsdaFixA(ql.Period(10, ql.TimeUnit.Years), curve)
vol = ql.ConstantSwaptionVolatility(
    today, ql.TARGET(), ql.BusinessDayConvention.Following, 0.20, ql.Actual365Fixed()
)
pricer = ql.AnalyticHaganPricer(vol, ql.YieldCurveModel.Standard, ql.make_quote_handle(0.0))
# compat: ql.GFunctionFactory.Standard

coupon = ql.CmsCoupon(pay, 1.0, start, pay, swap_index.fixing_days(), swap_index,
                      day_counter=ibor.day_counter())
coupon.set_pricer(pricer)
print(coupon.rate())

cms = ql.make_cms(ql.Period(5, ql.TimeUnit.Years), swap_index, ibor,
                  discount_curve=curve, pricer=pricer)
print(cms.NPV())
```

## Phase-11 CMS-spread

```python
cms10 = ql.EuriborSwapIsdaFixA(ql.Period(10, ql.TimeUnit.Years), curve, curve)
cms2 = ql.EuriborSwapIsdaFixA(ql.Period(2, ql.TimeUnit.Years), curve, curve)
spread = ql.SwapSpreadIndex("cms10y2y", cms10, cms2)  # make_swap_spread_index

tsr = ql.LinearTsrPricer(vol, ql.make_quote_handle(0.01), curve)
sp_pricer = ql.LognormalCmsSpreadPricer(
    tsr, ql.make_quote_handle(0.6), curve, integration_points=32
)
cpn = ql.CmsSpreadCoupon(pay, 10000.0, start, pay, spread.fixing_days(), spread,
                         day_counter=ql.Actual360())
cpn.set_pricer(sp_pricer)

capped = ql.CappedFlooredCmsSpreadCoupon(
    pay, 10000.0, start, pay, 2, spread, cap=0.03, day_counter=ql.Actual360()
)
```

## Phase-12 zero inflation / ZCIS

```python
hz = ql.RelinkableZeroInflationTermStructureHandle()
index = ql.UKRPI(hz)
# add historic RPI fixings…
helpers = [
    ql.ZeroCouponInflationSwapHelper(
        ql.make_quote_handle(0.0293), ql.Period(3, ql.TimeUnit.Months),
        maturity, ql.UnitedKingdom(), ql.BusinessDayConvention.ModifiedFollowing,
        ql.Thirty360(...), index, ql.CPIInterpolationType.Flat,  # compat: ql.CPI.Flat
    )
]
curve = ql.PiecewiseZeroInflationCurve(today, index.last_fixing_date(),
                                       ql.Frequency.Monthly, dc, helpers)
hz.link_to(curve)

zcis = ql.ZeroCouponInflationSwap(
    ql.SwapType.Payer, 1e6, today, maturity, cal, bdc, dc, 0.0293,
    index, ql.Period(3, ql.TimeUnit.Months), ql.CPIInterpolationType.Flat,
)
zcis.set_pricing_engine(nominal_curve)  # DiscountingSwapEngine
print(zcis.NPV(), zcis.fair_rate())
```

## When to stay on SWIG

Use the official `QuantLib` PyPI wheel if you need broad instrument coverage,
legacy examples, or full class hierarchies. Use `qlnb` when you want a smaller
nanobind surface, Stable ABI wheels, and NumPy-friendly helpers.

See also [packaging.md](packaging.md) for how wheels are built and
[free-threading.md](free-threading.md) for threaded / free-threaded notes.
