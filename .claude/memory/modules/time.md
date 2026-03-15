---
name: module_time
description: Architecture and state of quantlib/time module — enums, Date, Period, DayCounter, Calendar, Schedule
type: reference
---

# quantlib/time Module

**Status**: Core complete. 210 tests, 79% coverage.
**C++ reference**: `upstream/ql/time/`

## Architecture

All classes use **camelCase** method names to match QuantLib C++ API. Ruff rules N801/N802/N803/N806 are suppressed project-wide for this reason.

### Design Patterns
- **Bridge pattern** used by DayCounter, Calendar (base class holds an `_impl` ABC; subclasses provide concrete Impl)
- **IntEnum** used for all enumerations (allows integer comparison like C++)

## Files and Classes

### Enums (`quantlib/time/`)
| File | Class | Values | C++ source |
|---|---|---|---|
| `timeunit.py` | `TimeUnit(IntEnum)` | Days=0..Microseconds=8 | `ql/time/timeunit.hpp` |
| `frequency.py` | `Frequency(IntEnum)` | NoFrequency=-1..OtherFrequency=999 | `ql/time/frequency.hpp` |
| `weekday.py` | `Weekday(IntEnum)` | Sunday=1..Saturday=7 + Sun..Sat aliases | `ql/time/weekday.hpp` |
| `month.py` | `Month(IntEnum)` | January=1..December=12 + Jan..Dec aliases | `ql/time/date.hpp` |
| `businessdayconvention.py` | `BusinessDayConvention(IntEnum)` | Following=0..Nearest=6 | `ql/time/businessdayconvention.hpp` |
| `dategenerationrule.py` | `DateGeneration.Rule(IntEnum)` | Backward=0..CDS2015=9 | `ql/time/dategenerationrule.hpp` |

### Period (`quantlib/time/period.py`)
- `Period(n, units)` or `Period(frequency)` — (length, TimeUnit) pair
- Full algebra: `+`, `-`, `*`, `/`, `+=`, `-=`, `*=`, `/=`
- Comparison with exact and inexact (days-range) logic
- `normalize()` / `normalized()` — Months→Years, Days→Weeks
- Free functions: `years()`, `months()`, `weeks()`, `days()`
- `__str__` returns short format: `"3M"`, `"1Y"`

### Date (`quantlib/time/date.py`)
- Stores Excel-compatible serial number internally. Range: [367, 109574] = Jan 1 1901 to Dec 31 2199
- Constructors: `Date()`, `Date(serial)`, `Date(day, month, year)`
- Inspectors: `weekday()`, `dayOfMonth()`, `dayOfYear()`, `month()`, `year()`, `serialNumber()`
- Algebra: `date + days`, `date + Period`, `date - date` → int, `daysBetween(d1, d2)` → float
- `_advance(date, n, units)` — static method for Period arithmetic with month-end clamping
- Static: `isLeap(y)`, `minDate()`, `maxDate()`, `todaysDate()`, `startOfMonth()`, `endOfMonth()`, `isStartOfMonth()`, `isEndOfMonth()`, `nextWeekday()`, `nthWeekday()`
- Formatting: `__str__` ("January 15, 2006"), `iso_date()` ("2006-01-15"), `short_date()` ("01/15/2006")
- Lookup tables: `_YEAR_OFFSET`, `_MONTH_OFFSET`, `_MONTH_LEAP_OFFSET`, `_YEAR_IS_LEAP`, `_MONTH_LENGTH`
- **Known issue**: `__sub__` returns `Date | int` (union type). Pyright can't distinguish. Use `Date._advance()` directly in library code to avoid type errors.

### DayCounter (`quantlib/time/daycounter.py` + `daycounters/`)
Bridge pattern. Base: `DayCounter` with `Impl` ABC (`name()`, `dayCount()`, `yearFraction()`).

| File | Class | Formula | Notes |
|---|---|---|---|
| `actual360.py` | `Actual360(includeLastDay=False)` | dc/360 | Optional +1 day |
| `actual364.py` | `Actual364()` | dc/364 | |
| `actual365fixed.py` | `Actual365Fixed(convention)` | dc/365 | Standard, Canadian, NoLeap |
| `actual36525.py` | `Actual36525()` | dc/365.25 | |
| `actual366.py` | `Actual366()` | dc/366 | |
| `actualactual.py` | `ActualActual(convention)` | varies | ISDA, ISMA (Old impl), AFB |
| `thirty360.py` | `Thirty360(convention, terminationDate)` | 30/360 | USA, BondBasis, European, Italian, ISDA/German, NASD |
| `thirty365.py` | `Thirty365()` | 30/365 | |
| `business252.py` | `Business252()` | bdays/252 | Weekends only (no Calendar integration yet) |
| `one.py` | `OneDayCounter()` | always 1 | |
| `simpledaycounter.py` | `SimpleDayCounter()` | actual/actual year | |

### Calendar (`quantlib/time/calendar.py` + `calendars/`)
Bridge pattern. Base: `Calendar` with `Impl` ABC (`name()`, `isBusinessDay()`, `isWeekend()`).

Partial impls: `WesternImpl` (Sat/Sun weekends, Easter Monday table), `OrthodoxImpl`.

| File | Class | Notes |
|---|---|---|
| `nullcalendar.py` | `NullCalendar()` | Every day is business day |
| `weekendsonly.py` | `WeekendsOnly()` | Only Sat/Sun are holidays |
| `target.py` | `TARGET()` | ECB calendar. New Year, Good Friday, Easter Monday, Labour Day, Christmas, Dec 26 |
| `unitedstates.py` | `UnitedStates(market)` | Settlement, NYSE. All major US holidays |
| `unitedkingdom.py` | `UnitedKingdom(market)` | Settlement only |

Key Calendar methods:
- `adjust(date, convention)` — roll non-business days per convention
- `advance(date, n, unit, convention, endOfMonth)` — advance by business days or periods
- `businessDaysBetween(from, to, includeFirst, includeLast)`
- `addHoliday(d)` / `removeHoliday(d)` — runtime customization
- `holidayList(from, to)` / `businessDayList(from, to)`
- `startOfMonth(d)` / `endOfMonth(d)` — first/last business day of month

### Schedule (`quantlib/time/schedule.py`)
- `Schedule(dates=..., ...)` — list-based constructor
- `Schedule(effectiveDate=..., terminationDate=..., tenor=..., rule=..., ...)` — rule-based
- Generation rules implemented: **Backward**, **Forward**, **Zero**, **ThirdWednesday**
- Not yet implemented: Twentieth, TwentiethIMM, OldCDS, CDS, CDS2015
- `MakeSchedule` — fluent builder with `set_from()`, `to()`, `withTenor()`, `withCalendar()`, `withConvention()`, `withRule()`, `forwards()`, `backwards()`, `endOfMonth()`, `build()`
- Inspectors: `size()`, `dates()`, `date(i)`, `startDate()`, `endDate()`, `tenor()`, `rule()`, `isRegular(i)`, `calendar()`
- Iterable: `for d in schedule`

## Test Files
| File | Tests | What |
|---|---|---|
| `test_enums.py` | 14 | All enum values, aliases, member counts |
| `test_period.py` | 25 | Algebra, normalization, frequency roundtrip, conversions |
| `test_date.py` | 42 | Constructors, inspectors, full 109K-date consistency, algebra, Period arithmetic, formatting |
| `test_daycounters.py` | 59 | All conventions, 21 ISDA/ISMA/AFB test vectors |
| `test_calendar.py` | 47 | Base class, 5 calendars, adjust, advance, business days, holidays |
| `test_schedule.py` | 15 | Forward/Backward/Zero generation, MakeSchedule, daily schedule |

## What's Missing vs C++
- `Date.__sub__` type overload (returns `Date | int`, confuses pyright)
- Schedule: CDS-specific generation rules (Twentieth, TwentiethIMM, OldCDS, CDS, CDS2015)
- Business252: Calendar integration (currently weekends-only)
- UnitedStates: GovernmentBond, NERC, FederalReserve, SOFR, LiborImpact markets
- UnitedKingdom: Exchange, Metals markets
- Many other country calendars (40+ in C++)
- IMM/ECB/ASX date utilities
- `DateParser` for parsing date strings
