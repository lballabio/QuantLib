# Extending QuantLib Safely

Read this when adding a new instrument, term structure, calendar, or day
counter. Every recipe below ends with the same step: register the new files in
all three build systems, per
[`registering-new-files.md`](registering-new-files.md).

For the lifecycle and notification machinery these classes plug into, see
[`architecture.md`](architecture.md).

## New Instrument

1. Add the instrument class in `ql/instruments/` (override `isExpired()`,
   implement `setupArguments()`/`fetchResults()`).
2. Add an engine in `ql/pricingengines/<domain>/` using
   `GenericEngine<arguments, results>`.
3. Add tests in `test-suite/`.
4. Register all new files in CMake, Autotools, and the Visual Studio projects.

## New Term Structure

- Inherit the appropriate base (yield/vol/default/inflation).
- Implement the required `*Impl()` method.
- Choose fixed-date or moving-date reference mode.
- Register with dependent handles.

## New Calendar / DayCounter

- Calendar: add the implementation in `ql/time/calendars/` with `name()` and
  `isBusinessDay()`.
- DayCounter: add the implementation in `ql/time/daycounters/` with `dayCount()`
  and `yearFraction()`.

## Experimental Work

Prefer `ql/experimental/<topic>/` for unstable or new APIs, but keep coding
quality and tests high. Interfaces there are not guaranteed to stay compatible
across releases.
