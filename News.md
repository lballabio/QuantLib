Changes for QuantLib 1.24:
==========================

QuantLib 1.24 includes 25 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/20?closed=1>.

Portability
-----------

- Overhauled the CMake build system (thanks to Philip Kovacs).  Among
  other things, it now allows to specify the available configuration
  options from the `cmake` invocation and adds the required Boost
  libraries accordingly.

Instruments
-----------

- Avoid callable-bond mispricing when a call date is close but not equal
  to a coupon date (thanks to Ralf Konrad for the fix and to GitHub user
  @aichao for the analysis).
  See <https://github.com/lballabio/QuantLib/issues/930> for details.

- A new `RiskyBondEngine` is available for bonds (thanks to Lew Wei
  Hao).  It prices bonds based on a risk-free discount cure and a
  default-probability curve used to assess the probability of each
  coupon payment.  It makes accessible to all bonds the calculations
  previously available in the experimental `RiskyBond` class.

Cashflows
---------

- The choice between par and indexed coupons was moved to
  `IborCouponPricer` (thanks to Peter Caspers).  This also made it
  possible to override the choice locally when building a
  `VanillaSwap` or a `SwapRateHelper`, so that coupons with both
  behaviors can now be used at the same time.

Term structures
---------------

- Cross-currency basis swap rate helpers now support both
  constant-notional and marked-to-market swaps (thanks to Marcin
  Rybacki).

Date/time
---------

- Added Chilean calendar (thanks to Anubhav Pandey).

- Added new `ThirdWednesdayInclusive` date-generation rule that also
  adjusts start and end dates (thanks to Lew Wei Hao).

Patterns
--------

- Overhauled `Singleton` implementation (thanks to Peter Caspers).
  Singletons are now initialized in a thread-safe way when sessions
  are enabled, global singletons (that is, independent of sessions)
  were made available, and static initialization was made safer.

Test suite
----------

- Sped up some of the longer-running tests (thanks to Mohammad Shojatalab).

Deprecated features
-------------------

- Deprecated default constructor for the U.S. calendar; the desired
  market should now be passed explicitly.

- Deprecated the `nominalTermStructure` method and the corresponding
  data member in inflation term structures.  Any object needing the
  nominal term structure should have it passed explicitly.

- Deprecated the `termStructure_` data member in
  `BlackCalibrationHelper`.  It you're inheriting from
  `BlackCalibrationHelper` and need it, declare it in your derived
  class.

- Deprecated the `createAtParCoupons`, `createIndexedCoupons` and
  `usingAtParCoupons` methods of `IborCoupon`, now moved to a new
  `IborCoupon::Settings` singleton (thanks to Philip Kovacs).

- Deprecated the `conversionType` and `baseCurrency` static data
  members of `Money`, now moved to a new `Money::Settings` singleton
  (thanks to Philip Kovacs).

- Removed features deprecated in version 1.19: the `BMAIndex`
  constructor taking a calendar, the `AmericanCondition` and
  `ShoutCondition` constructors taking an option type and strike, the
  `CurveDependentStepCondition` class and the
  `StandardCurveDependentStepCondition` typedef, the
  `BlackCalibrationHelper` constructor taking a yield term structure,
  the various inflation term structure constructors taking a yield
  term structure, the various yield term constructors taking a vector
  of jumps but not specifying a reference date.


Thanks go also to Mickael Anas Laaouini, Jack Gillett, Bojan Nikolic
and Klaus Spanderen for smaller fixes, enhancements and bug reports.

