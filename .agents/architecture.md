# Architecture Deep Dive

Companion to [`AGENTS.md`](../AGENTS.md). Read this when you need to understand
how pricing, notification, and cash-flow machinery fit together, rather than
just where files live.

## 1. Instrument-Engine Lifecycle

When you call `instrument.NPV()`:

1. `Instrument::calculate()` checks `isExpired()` → if expired, calls
   `setupExpired()`.
2. If `calculated_ == true` (no dependency change), cached results are reused
   immediately.
3. Otherwise, `Instrument::performCalculations()` runs the engine cycle:
   `engine_->reset()` → `setupArguments()` → `validate()` →
   `engine_->calculate()` → `fetchResults()`

Engines receive all data through the `arguments` struct and write all outputs to
the `results` struct. They never hold references to the instrument.

**Recalculation triggers** — any of these call `notifyObservers()`, ultimately
clearing `calculated_` via `LazyObject::update()`:

- **Quote value change** — `SimpleQuote::setValue()`.
- **Handle relink/forward** — `Handle<T>::Link::linkTo()` / `update()`.
- **Evaluation date change** — `Settings::evaluationDate() = newDate`, via
  `ObservableValue::operator=()`.
- **Term-structure update** — `TermStructure::update()` (sets
  `updated_ = false`).
- **Index update** — `Index::update()`.
- **Engine replacement** — `Instrument::setPricingEngine()` (re-registers the
  engine and calls `update()`).

**`calculated_` flag** (in `ql/patterns/lazyobject.hpp`): set `true` **before**
`performCalculations()` (to break bootstrap cycles), reset to `false` on
`update()` or if `performCalculations()` throws. `recalculate()` forces an
immediate recomputation.

## 2. CashFlow and Coupon Subsystem

```mermaid
graph TD
    Event["Event"] --> CashFlow
    LazyObject["LazyObject"] --> CashFlow
    CashFlow --> SimpleCashFlow
    CashFlow --> Coupon
    CashFlow --> InflationCoupon
    CashFlow --> IndexedCashFlow
    Coupon --> FixedRateCoupon
    Coupon --> FloatingRateCoupon
    FloatingRateCoupon --> IborFamily["Ibor/OIS/CMS/CappedFloored..."]
```

- `CashFlow` API: `date()`, `amount()`, `hasOccurred()`, `exCouponDate()`.
- `Leg` is a type alias, not a class: `std::vector<ext::shared_ptr<CashFlow> >`
  (`ql/cashflow.hpp`). There is nothing to derive from or extend.
- `Coupon` adds accrual data (`nominal()`, `rate()`, `dayCounter()`,
  `accrualPeriod()`).
- `FloatingRateCoupon` delegates pricing to `FloatingRateCouponPricer`
  (`ql/cashflows/couponpricer.hpp`).
- Utility analytics are in `ql/cashflows/cashflows.hpp` (`npv`, `bps`, `yield`,
  `duration`, `convexity`, `zSpread`, ...).

## 3. Design Patterns You Must Respect

- **Observer/Observable** (`ql/patterns/observable.hpp`) — market-data updates
  invalidate downstream pricing caches.
- **LazyObject** (`ql/patterns/lazyobject.hpp`) — cache plus
  recompute-on-demand behavior.
- **Handle/RelinkableHandle** (`ql/handle.hpp`) — relinking propagates
  notifications through the dependency graph.
- **Singleton** (`ql/patterns/singleton.hpp`) — process-global by default;
  per-session when sessions are enabled (`Settings`, `IndexManager`, lazy
  defaults).
- **Bridge/Pimpl** (`ql/time/calendar.hpp`, `ql/time/daycounter.hpp`) —
  swappable behavior behind stable interfaces.
- **Visitor** (`ql/patterns/visitor.hpp`) — runtime dispatch for
  payoff/instrument hierarchies.

`LazyObject` details worth remembering (`ql/patterns/lazyobject.hpp`):

- Inherits **both** `Observable` and `Observer`.
- `calculate()` sets `calculated_ = true` before `performCalculations()` to
  avoid recursive blowups.
- On exception, `calculated_` is reset to `false`.
- `update()` invalidates cache and forwards notifications per object/default
  policy.
- `freeze()`/`unfreeze()` can intentionally suppress/release notifications.
- `LazyObject::Defaults` changes apply to **newly created** lazy objects.

## 4. About `ql/experimental/`

Treat `ql/experimental/*` as unstable API: useful, often production-grade in
parts, but not guaranteed to keep interface compatibility across releases.
