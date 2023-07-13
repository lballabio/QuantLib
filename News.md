Changes for QuantLib 1.31:
==========================

QuantLib 1.31 includes a record 68 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/28?closed=1>.


Portability
-----------

- **Future end of support:** as announced in the notes for the
  previous release, after this release using `std::tuple`,
  `std::function` and `std::bind` (instead of their `boost`
  counterparts) will become the default.  If you're using `ext::tuple`
  etc. in your code (which is suggested), this should be a transparent
  change.  If not, you'll still be able to choose the `boost` versions
  via a configure switch for a while; but we do suggest you start
  using `ext::tuple` etc. in the meantime.

- The cmake build now creates (but doesn't install) a
  `quantlib-config` script that can be used to retrieve flags for
  compiling QuantLib-dependent projects; thanks to Christian
  Köhnenkamp (@kohnech).

- A number of Boost classes and functions only used internally were
  replaced by their standard-library equivalent; thanks to Jonathan
  Sweemer (@sweemer).


Patterns
--------

- **Optional change of behavior:** by default, the `LazyObject` class
  forwards only one notification after recalculating and silently
  ignores the others.  In some edge cases, this could lead to objects
  not being updated.  It's now possible to enable a different behavior
  where all notifications are forwarded; the new behavior can be
  chosen at compile time via the configure option
  `--disable-faster-lazy-objects` (or disabling
  `QL_FASTER_LAZY_OBJECTS` in cmake or `userconfig.hpp`) or at run
  time by calling
  `LazyObject::Defaults::instance().alwaysForwardNotifications()`.
  This might cause a slow down, so you're invited to try it out and
  report on the mailing list.  If there are no problems, the new
  behavior might become the default in future releases.  Also, a new
  configure option `--enable-throwing-in-cycles` (`QL_THROW_IN_CYCLES`
  in cmake or `userconfig.hpp`) is optionally available; when both
  this option and the new behavior are enabled, notifications cycles
  involving a lazy object will throw an exception.  It is suggested to
  try enabling the option and removing such loops, if any.  Thanks to
  Peter Caspers (@pcaspers) for the change and to Ralf Konrad
  (@ralfkonrad), Jonathan Sweemer (@sweemer) and GitHub user
  @djkrystul for feedback.


Date/time
---------

- **Change of behavior:** when the end-of-month option is true, the
  constructor of a schedule no longer adjust to the end of their month
  the effective date and the termination date if they were passed
  explicitly.  Thanks to Hristo Raykov (@HristoRaykov).

- Added separate US SOFR calendar to manage days that are business
  days for the US government bond market but in which SOFR doesn't
  fix; for instance, Good Friday 2023 (@lballabio).  Thanks to Tom
  Anderson (@tomwhoiscontrary) for reporting the issue.

- Fixed some rolling rules for South Korean calendar; thanks to Jonghee
  Lee (@nistick21).

- Fixed incorrect 2023 holidays for Hong Kong calendar; thanks to
  Fredrik Gerdin Börjesson (@gbfredrik).

- Added Hong Kong holidays for 2021-2024; thanks to Rémy Frèrebeau
  (@rfrerebe-stx) and Binrui Dong (@BrettDong).

- Added Singapore holidays for 2019-2023; thanks to Rémy Frèrebeau
  (@rfrerebe-stx).

- Added Indian holidays for 2021-2025; thanks to Fredrik Gerdin
  Börjesson (@gbfredrik).

- Added Taiwanese holidays for 2020-2023; thanks to @jsmx.

- Added a few election days for South African and South Korean
  calendar; thanks to Fredrik Gerdin Börjesson (@gbfredrik).

- Updated Danish calendar; starting in 2024, General Prayer Day will
  no longer be a holiday.  Thanks to Fredrik Gerdin Börjesson
  (@gbfredrik).

- Fixed a few holidays in Finland and Singapore calendars; Thanks to
  Fredrik Gerdin Börjesson (@gbfredrik).

- More day counters (Act/364, Act/365.25, Act/366) now take into
  account intraday resolution when enabled; thanks to Klaus Spanderen
  (@klausspanderen).


Cash flows
----------

- The accrued amount for CPI coupons is now correctly based on the
  index ratio at settlement date.  An inspector for retrieving the
  index ratio at a given date was also added (@lballabio).

- Enabled the use of normal volatilities in Hagan pricer for CMS
  coupons; thanks to Andre Miemiec (@amiemiec).

- Floating-rate coupons are now lazy; thanks to Peter Caspers
  (@pcaspers).


Indexes
-------

- When passed a tenor of 7 or 14 business days, interest-rate indexes
  would wrongly convert it to 1 or 2 weeks.  This is now fixed
  (@lballabio).  Thanks to Eugene Toder (@eltoder) for reporting the
  issue.

- Added DESTR and SWESTR indexes; thanks to Fredrik Gerdin Börjesson
  (@gbfredrik).

- Added CORRA index; thanks to @AND2797.

- When an YoY inflation index is calculated as a ratio, the underlying
  inflation index is available through an inspector and its fixings
  are used to calculate the fixing of the YoY index (@lballabio).


Instruments
-----------

- Instruments now register automatically with the global evaluation
  date and are notified when it changes.  This makes sense in general
  (if the evaluation date changes, you probably want to recalculate)
  and can also help avoid some edge cases when lazy objects only
  forward their first notification (@lballabio).

- Allowed passing a schedule without a regular tenor to callable
  fixed-rate bonds; thanks to Hristo Raykov (@HristoRaykov) for the
  fix and to @OleBueker for reporting the issue.

- Reorganized the constructors of FRA instruments; thanks to Jake Heke
  (@jakeheke75).


Term structures
---------------

- Ensures that upfront CDS helpers update correctly when the global
  evaluation date changes; thanks to Andrea Pellegatta (@andrea85p)
  for the fix and to @bkhoor for reporting the issue.

- Allow more maturities for SOFR quarterly contract in SOFR futures
  rate helper; thanks to Jake Heke (@jakeheke75).

- Added constructor for date-dependent strikes to StrippedOptionlet;
  thanks to Peter Caspers (@pcaspers).


Test suite
----------

- Global settings (such as the evaluation date) are now restored and
  index fixings are now cleaned automatically at the end of each test
  case, making it unnecessary to clean them up manually.  Thanks to
  Eugene Toder (@eltoder).

- The parallel unit-test runner now passes the `--run_test=<filter>`
  option down to the underlying Boost.Test implementation.  Thanks to
  Eugene Toder (@eltoder).


Deprecated features
-------------------

- **Removed** features deprecated in version 1.26:
  - The `CPICoupon` constructor taking a number of fixing days and its
    `indexObservation`, `adjustedFixing` and `indexFixing(date)` methods.
  - The `CPICashFlow` constructor taking a fixing date.
  - The `withFixingDays` methods of `CPILeg`.
  - The `ZeroInflationCashFlow` constructor taking a calendar and
    business-day convention.
  - The `LsmBasisSystem::PolynomType` typedef and the
    `MakeMCAmericanEngine::withPolynomOrder` method.
  - The `Observer::set_type` and `Observable::set_type` typedefs.
  - The `Curve` class.
  - The `LexicographicalView` class.
  - The `Composite` class.
  - The `DriftTermStructure` class.

- Deprecated the various `time_iterator` and `value_iterator` types in
  `TimeSeries`, as well as methods returning them.  The more general
  `const_iterator` and `const_reverse_iterator` types can be used
  instead.

- Deprecated the constructors of `CPICoupon` taking a spread, as well
  as its `spread` method, its protected `spread_` data member, and the
  `withSpreads` methods of `CPILeg`.

- Deprecated the `adjustedFixing` method and the protected `spread_`
  data member of `CPICouponPricer`.

- Renamed `BlackVanillaOptionPricer` to `MarketQuotedOptionPricer` and
  deprecated the old name.

- Deprecated a couple of constructors of `ForwardRateAgreement`.

- Deprecated the constructor of `YoYInflationIndex` taking a `ratio`.
  Also, deprecated explicit classes for YoY ratio indexes
  `YYGenericCPIr`, `YYAUCPIr`, `YYEUHICPr`, `YYFRHICPr`, `YYUKRPIr`,
  `YYUSCPIr` and `YYZACPIr`.

- Deprecated the `base`, `increment`, `decrement`, `advance` and
  `distance_to` methods of the `step_iterator` class.


**Thanks go also** to Jonathan Sweemer (@sweemer), Jose Garcia
(@j053g), Jake Heke (@jakeheke75), Eugene Toder (@eltoder), Binrui
Dong (@BrettDong), the Xcelerit Dev Team (@xcelerit-dev), Ralf Konrad
(@ralfkonrad), Fredrik Gerdin Börjesson (@gbfredrik) and Tom Anderson
(@tomwhoiscontrary) for a number of smaller fixes and improvements.
