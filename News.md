Changes for QuantLib 1.30:
==========================

QuantLib 1.30 includes 34 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/27?closed=1>.


Portability
-----------

- **Future end of support:** as announced in the notes for the
  previous release, after this release and the next, using
  `std::tuple`, `std::function` and `std::bind` (instead of their
  `boost` counterparts) will become the default.  If you're using
  `ext::tuple` etc. in your code (which is suggested), this should be
  a transparent change.  If not, you'll still be able to choose the
  `boost` versions via a configure switch for a while; but we do
  suggest you start using `ext::tuple` etc. in the meantime.

- CMake builds now use a stricter warning level by default; thanks to
  Ralf Konrad (@ralfkonrad).

- Is it now possible to use `std::any` and `std::optional` (and the
  related `std::any_cast` and `std::nullopt`) instead of their `boost`
  counterparts by setting new compilation switches; thanks to Jonathan
  Sweemer (@sweemer).  Using the `std` classes requires C++17.  We
  expect the `boost` classes to remain the default for a while, but in
  the meantime we encourage to start using `ext::any` and
  `ext::optional` in preparation for a new default.


Date/time
---------

- Good Friday 2023 is now a business day for the US government bond
  calendar; thanks to Anastasiia Shumyk (@ashumyk).

- Added specialized Australian calendar for ASX; thanks to Trent
  Maetzold (@trentmaetzold).

- Fixed Turkish holidays between 2019 and 2023; thanks to Fredrik
  Gerdin Börjesson (@gbfredrik).

- Added a few missing holidays to Danish calendar; thanks to Fredrik
  Gerdin Börjesson (@gbfredrik).

- Added the Matariki holiday to the New Zealand calendar; thanks to
  g.t. (@jakeheke75).


Cashflows
---------

- Added a new equity cash flow class to model equity legs in total
  return swaps; thanks to Marcin Rybacki (@marcin-rybacki).  Quanto
  pricing is also supported.

- Added an overloaded constructor for CPI coupons that allows to
  specify a base date instead of a base CPI value; thanks to Matthias
  Groncki (@mgroncki).


Instruments
-----------

- Added a new total-return swap; thanks to Marcin Rybacki
  (@marcin-rybacki).  An equity-index class was also added to support
  this instrument.

- The analytic engine for barrier options would return NaN for low
  values of volatility; this is now fixed (@lballabio).

- The `VanillaOption` and `BarrierOption` classes can now be used to
  model vanilla and barrier options with discrete dividends; the
  future dividends (not being part of the terms and conditions of the
  contract) should be passed to the pricing engine instead (@lballabio).

- Added analytical Greeks to Bjerksund-Stensland engine; thanks to
  Klaus Spanderen (@klausspanderen).


Indexes
-------

- Added UKHICP inflation index; thanks to Fredrik Gerdin Börjesson
  (@gbfredrik).


Term structures
---------------

- Renamed `SwaptionVolCube1`, `SwaptionVolCube1x`, `SwaptionVolCube1a`
  and `SwaptionVolCube2` to `SabrSwaptionVolatilityCube`,
  `XabrSwaptionVolatilityCube`, `NoArbSabrSwaptionVolatilityCube` and
  `InterpolatedSwaptionVolatilityCube`, respectively; thanks to
  Ignacio Anguita (@IgnacioAnguita).  The old names are deprecated but
  still available for a few releases.

- Ensure that inflation curves are re-bootstrapped correctly when
  seasonality is added (@lballabio).


Models
------

- Moved the Heston SLV model from experimental to main; thanks to
  Klaus Spanderen (@klausspanderen).


Math
----

- Added a few overloads to Array and Matrix operators taking rvalue
  references for increased speed; thanks to Jonathan Sweemer (@sweemer).


Deprecated features
-------------------

- **Removed** features deprecated in version 1.24:
  - the protected `spreadLegValue_` data member of `BlackIborCouponPricer`;
  - the `WulinYongDoubleBarrierEngine` alias for `SuoWangDoubleBarrierEngine`;
  - the `settlementDate`, `incomeDiscountCurve`, `spotIncome`,
    `spotValue`, `impliedYield` and `forwardValue` methods of
    `ForwardRateAgreement`, as well as its protected
    `underlyingIncome_`, `underlyingSpotValue_`, `settlementDays_`,
    `payoff_` and `incomeDiscountCurve_` data members;
  - constructors for `InflationTermStructure`,
    `ZeroInflationTermStructure`, `InterpolatedZeroInflationCurve`,
    `PiecewiseZeroInflationCurve` taking an `indexIsInterpolated`
    parameter;
  - the `indexIsInterpolated` method of `InflationTermStructure` and
    its protected `indexIsInterpolated_` data member;
  - some overloaded constructors of `SofrFutureRateHelper`.

- Deprecated the `DividendVanillaOption` and `DividendBarrierOption`
  classes; use `VanillaOption` and `BarrierOption` instead (see
  above).

- Deprecated the constructor of `AnalyticDividendEuropeanEngine` that
  takes no dividend information; use the other overload instead.

- Deprecated the names `SwaptionVolCube1`, `SwaptionVolCube1x`,
  `SwaptionVolCube1a` and `SwaptionVolCube2` (see above).

- Deprecated the protected `setCommon` method of
  `CappedFlooredYoYInflationCoupon`.


**Thanks go also** to Jonathan Sweemer (@sweemer), the Xcelerit Dev
Team (@xcelerit-dev), Fredrik Gerdin Börjesson (@gbfredrik), Klaus
Spanderen (@klausspanderen) and Peter Caspers (@pcaspers) for a number
of smaller fixes and improvements, and to Matthias Groncki (@mgroncki)
and @lukey8767 for raising issues.

