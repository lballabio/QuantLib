Changes for QuantLib 1.37:
==========================

QuantLib 1.37 includes 26 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/35?closed=1>.


Portability
-----------

- **Future change of default:** as already announced, in the next
  release we're going to switch the default for `ext::any` and
  `ext::optional` from the Boost implementation to the standard one.

Dates and calendars
-------------------

- Added closure for President Carter's funeral to the NYSE calendar;
  thanks to Dirk Eddelbuettel (@eddelbuettel).

- Added distinct Wellington and Auckland variants for New Zealand
  calendar (@lballabio).


Indexes
-------

- Improved the performance of the `addFixing` and `addFixings` method
  in the `Index` class; thanks to Peter Caspers (@pcaspers).

- Added the KOFR index; thanks to Jongbong An (@jongbongan).


Instruments and pricing engines
-------------------------------

- Added Choi pricing engine for Asian options; thanks to Klaus
  Spanderen (@klausspanderen).

- Passing a risk-free overnight index to an asset swap now implies
  using OIS-like coupons (@lballabio).

- Added Bjerksund-Stensland, Operator-Splitting, Deng-Li-Zhou, Choi
  and n-dim PDE engines for spread options; thanks to Klaus Spanderen
  (@klausspanderen).

- Deng-Li-Zhou, Choi and n-dim PDE engines for basket options; thanks
  to Klaus Spanderen (@klausspanderen).


Term structures
---------------

- Better upper and lower bounds for global bootstrap; thanks to Eugene
  Toder (@eltoder).

- Fitted bond curves can now be passed precomputed parameters without
  the need for bond helpers (@lballabio).

- Use correct guess in SABR swaption vol cube (@lballabio).

- OIS rate helpers can now be passed a date-generation rule; thanks to
  Sotirios Papathanasopoulos (@sophistis42).

- Swap rate helpers can now be passed explicit start and end dates;
  thanks to Eugene Toder (@eltoder).

- OIS rate helpers can now be passed explicit start and end dates,
  making a distinct `DatedOISRateHelper` class unnecessary; thanks to
  Eugene Toder (@eltoder).


Cash flows
----------

- Added new `MultipleResetsCoupon` and `MultipleResetsLeg` classes to
  manage coupons with multiple resets (@lballabio).  They fix and
  replace `SubPeriodsCoupon` and `SubPeriodsLeg`.


Deprecated features
-------------------

- **Removed** features deprecated in version 1.32:
  - the `FixedRateBondForward` class;
  - the `SampledCurve` and `SampledCurveSet` classes;
  - the `StepConditionSet` and `BoundaryConditionSet` classes;
  - the `ParallelEvolver` and `ParallelEvolverTraits` classes;
  - the `FDVanillaEngine` and `FDMultiPeriodEngine` classes;
  - the `BSMTermOperator`, `StandardFiniteDifferenceModel`,
    `StandardSystemFiniteDifferenceModel` and `StandardStepCondition`
    typedefs;
  - the `QL_NULL_FUNCTION` macro;
  - the overloads of `DigitalCmsLeg::withReplication` ,
    `DigitalCmsSpreadLeg::withReplication` and
    `DigitalIborLeg::withReplication` taking no arguments;
  - the empty headers `analyticamericanmargrabeengine.hpp`,
    `analyticcomplexchooserengine.hpp`,
    `analyticcomplexchooserengine.hpp`,
    `analyticcompoundoptionengine.hpp`,
    `analyticeuropeanmargrabeengine.hpp`,
    `analyticsimplechooserengine.hpp`, `complexchooseroption.hpp`,
    `compoundoption.hpp`, `margrabeoption.hpp` and
    `simplechooseroption.hpp` in the `ql/experimental/exoticoptions`
    folder;
  - the empty header `ql/experimental/termstructures/multicurvesensitivities.hpp`;
  - the empty headers `pdeshortrate.hpp` and `shoutcondition.hpp` in
    the `ql/methods/finitedifferences` folder;
  - the empty header `ql/models/marketmodels/duffsdeviceinnerproduct.hpp`;
  - the empty headers `fdconditions.hpp`, `fddividendengine.hpp` and
    `fdstepconditionengine.hpp` in the `ql/pricingengines/vanilla`
    folder.

- Deprecated the `SubPeriodsCoupon`, `SubPeriodsPricer`,
  `AveragingRatePricer` and `CompoundingRatePricer` classes; renamed
  to `MultipleResetsCoupon`, `MultipleResetsPricer`,
  `AveragingMultipleResetsPricer` and
  `CompoundingMultipleResetsPricer`, respectively.

- Deprecated the `SubPeriodsLeg` class; use `MultipleResetsLeg` instead.

- Deprecated the `MultipleResetsCoupon` constructor without a reset
  schedule; use the other constructor.

- Deprecated the `calendar`, `price`, `addQuote`, `addQuotes`,
  `clearQuotes`, `isValidQuoteDate` and `quotes` methods in the
  `CommodityIndex` class; use `fixingCalendar`, `fixing`, `addFixing`,
  `addFixings`, `clearFixings`, `isValidFixingDate` and `timeSeries`
  instead.

- Deprecated the experimental `SpreadOption` and `KirkSpreadOptionEngine`
  classes; use `BasketOption` and `KirkEngine` instead.

- Deprecated the `TransformedGrid` and `LogGrid` classes and the
  `CenteredGrid`, `BoundedGrid` and `BoundedLogGrid` functions; use
  the new FD framework instead.

- Deprecated the `PdeOperator` and `BSMOperator` classes; use the new
  FD framework instead.

- Deprecated the `PdeSecondOrderParabolic`, `PdeConstantCoeff`,
  `PdeBSM` and `GenericTimeSetter` classes; use the new FD framework
  instead.

- Deprecated the `hasHistory`, `getHistory`, `clearHistory`,
  `hasHistoricalFixing` and `setHistory` in the `IndexManager` class;
  use `Index::hasHistoricalFixing`, `Index::timeSeries`,
  `Index::clearFixings`, `Index::hasHistoricalFixing` and
  `Index::addFixings` instead.

- Deprecated the `notifier` method in the `IndexManager` class;
  register with the relevant index instead.

- Deprecated one of the `AssetSwap` constructors; use the other overload.

- Deprecated the `fcn` and `jacFcn` methods in the
  `LevenbergMarquardt` class; they are for internal use only.

- Deprecated the `indexIsInterpolated` parameter in YoY inflation
  curve constructors; use another overload.  Fixings will be
  interpolated by coupons instead, so curves and indexes will only be
  asked for fixing at the start of a month.

- Deprecated the `indexIsInterpolated` method and the
  `indexIsInterpolated_` data member in the
  `YoYInflationTermStructure` class.

- Deprecated the `DatedOISRateHelper` class; use `OISRateHelper`
  instead.


**Thanks go also** to Eugene Toder (@eltoder), Ben Watson (@sonben)
and the XAD team (@auto-differentiation-dev) for miscellaneous
smaller fixes, improvements or reports.
