Changes for QuantLib 1.34:
==========================

QuantLib 1.34 includes 35 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/32?closed=1>.


Portability
-----------

- **Future end of support:** as announced in release 1.32, we're
  targeting next release (1.35) as the last to support Visual C++
  2015, g++ up to version 6.x, and clang up to version 4; support for
  those compilers will be dropped in release 1.36, about six months
  from now.  From that point onwards, this will allows us to enable
  the use of C++17 in the code base.
  Also, given the testing environments available on GitHub actions,
  clang 5 is already no longer available to us for testing, and in a
  while the same will hold for clang 6 and g++ 7.  Therefore, it is
  suggested to upgrade to a newer version if possible.

- **Future end of support:** at the same time as the above, we'll also
  remove the configure switch that allows to use `boost::tuple`,
  `boost::function` and `boost::bind` instead of their `std`
  counterparts; the `std` classes are already the default since
  release 1.32.

- Generate and install pkg-config files in CMake builds; thanks to
  GitHub user @jez6.


Dates and calendars
-------------------

- Prevent `Calendar::advance` from returning the business end of month
  (instead of the calendar end) when `endOfMonth` is `true` and
  `convention` is `Unadjusted`; thanks to GitHub user @DeimosXing.

- Add good Friday holiday for SOFR fixing; thanks to GitHub user
  @PaulXiCao.

- Properly restrict São Paulo city holiday to years before 2022;
  thanks to Marco Bruno Ferreira Vasconcellos (@marcobfv).

- Update holidays for 2023 and 2024 in calendars for India, Thailand,
  Singapore and South Africa; thanks to Fredrik Gerdin Börjesson
  (@gbfredrik).


Cash flows
----------

- Fixed a couple of cases in which notifications were not forwarded
  properly; thanks to GitHub user @djkrystul for the heads-up.

- Fixed past payment dates and added support for OIS in
  `LinearTsrPricer`; thanks to Peter Caspers (@pcaspers).


Instruments
-----------

- Swaptions can now take an OIS as underlying; thanks to Guillaume
  Horel (@thrasibule) and Peter Caspers (@pcaspers).  So far, only
  `BlackSwaptionEngine` manages OIS explicitly; other engines might
  work and return approximated values.

- More methods in `MakeOIS` and `MakeVanillaSwap`; thanks to Eugene Toder
  (@eltoder).

- More methods in the `BondFunctions` class now support either clean or
  dirty prices; thanks to Francois Botha (@igitur).

- The `basisPointValue` and `yieldValueBasisPoint` methods in
  `BondFunctions` didn't always manage the settlement date correctly;
  this is now fixed (thanks to GitHub user @jez6).

- Add `Custom` to `Futures::Type` enumeration to allow passing custom
  dates to futures; thanks to Eugene Toder (@eltoder).


Term structures
---------------

- Inflation curves can now be built passing an explicit base date
  (corresponding to the last published fixing) instead of an
  observation lag (@lballabio).

- Fixed calculation of year fraction under Actual/365 Canadian
  convention in `FuturesRateHelper`; thanks to GitHub user @PaulXiCao.

- Fixed settlement date calculation in cross-currency basis-swap rate
  helpers in some cases; thanks to Marcin Rybacki (@marcin-rybacki)
  for the fix and to Aleksis Ali Raza for the heads-up.


Math
----

- Handle non-equidistant grids and arbitrary dimensions in Laplace
  interpolation; thanks to Peter Caspers (@pcaspers).


Deprecated features
-------------------

- **Removed** features deprecated in version 1.29:
  - The `argument_type`, `first_argument_type`, `second_argument_type`
    and `result_type` typedefs in several classes;
  - The overloads of zero-rate inflation index constructors taking an
    `interpolated` argument;
  - The `interpolated` method and the protected `interpolated_` data
    member in `InflationIndex`;
  - The overload of `CashFlows::npvbps` taking the result by reference;
  - The protected `rateCurve_` method in `InflationCouponPricer`;
  - The `ThreadKey` typedef;
  - The empty header `ql/experimental/credit/riskybond.hpp`.

- Deprecated the constructors of `InflationTermStructure`,
  `ZeroInflationTermStructure`, `YoYInflationTermStructure`,
  `InterpolatedZeroInflationCurve`, `InterpolatedYoYInflationCurve`,
  `PiecewiseZeroInflationCurve` and `PiecewiseYoYInflationCurve`
  taking an observation lag; use the overloads taking an explicit base
  date instead.

- Deprecated the `Bond::yield`, `BondFunctions::atmRate`,
  `BondFunctions::yield` and `BondFunctions::zSpread` overloads taking
  a clean price as a number; use the overloads taking a `Bond::Price`
  instead.

- Deprecated the `InflationTermStructure::setSeasonality` overload
  taking no arguments; use the overload taking a pointer and pass an
  empty one to remove seasonality.

- Deprecated the `InflationTermStructure::setBaseRate` method; set
  `baseRate_` directly if needed.

- Deprecated the `Swaption::underlyingSwap` and
  `SwaptionHelper::underlyingSwap` methods; use `underlying` instead.

- Deprecated the broken `FixedRateBondHelper::fixedRateBond` and
  `CPIBondHelper::cpiBond` methods and the corresponding
  `fixedRateBond_` and `cpiBond_` data members.


**Thanks go also** to Isuru Fernando (@isuruf), Viktor Zhou
(@yyuuhhjjnnmm), Stephen Dacek (@sdacek), Yi Jiang (@yjian012),
Jonathan Sweemer (@sweemer), Eugene Toder (@eltoder), the XAD team
(@auto-differentiation-dev) and GitHub user @PaulXiCao and @klin333
for miscellaneous fixes, improvements or reports.
