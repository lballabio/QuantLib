Changes for QuantLib 1.36:
==========================

QuantLib 1.36 includes 34 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/34?closed=1>.


Portability
-----------

- **New minimum C++ standard:** starting from this release, a compiler
  supporting C++17 is required.  Passing `--enable-std-classes` to
  `configure` now causes `std::any` and `std::optional` to be used.

- **End of support:** related to the above, and as announced since
  release 1.32, this release drops support Visual C++ 2015, g++ up to
  version 6.x, and clang up to version 4.  Also, given the testing
  environments available on GitHub actions, clang 5 and 6 are no
  longer available to us for testing, and the same holds for g++ 7.
  Therefore, it is suggested to upgrade to a newer version if
  possible.

- **End of support:** this release also removes the configure switch
  that allowed to use `boost::tuple`, `boost::function` and
  `boost::bind` instead of their `std` counterparts; the `std` classes
  were already the default since release 1.32.  The corresponding
  classes in the `ext` namespace are now deprecated.

- **Future change of default:** in a couple of releases, we're going
  to switch the default for `ext::any` and `ext::optional` from the
  Boost implementation to the standard one.


Dates and calendars
-------------------

- Added `startOfMonth` and `isStartOfMonth` methods to both `Date` and
`Calendar`; thanks to Francois Botha (@igitur).

- Added specialized Warsaw Stock Exchange (WSE) calendar to Poland;
  thanks to Marcin Bogusz (@marcinfair).

- Added a new one-off holiday to South Korean calendar; thanks to
  Jongbong An (@jongbongan).


Cash flows
----------

- Made` OvernightIndexedCouponPricer` public and renamed to
  `CompoundingOvernightIndexedCouponPricer`, and moved
  `ArithmeticAveragedOvernightIndexedCouponPricer` from experimental
  to core library; thanks to Ralf Konrad Eckel (@ralfkonrad).


Indexes
-------

- **Possibly breaking:** inherited the `Index` class from `Observer`
  and added a virtual `pastFixing` method.  If you inherited a class
  from both `Index` and `Observer`, change your code to avoid
  inheriting twice from `Observer`.  Thanks to Ralf Konrad Eckel
  (@ralfkonrad).

- Added currency information to `EquityIndex`; thanks to Ralf Konrad
  Eckel (@ralfkonrad).


Inflation
---------

- Inflation indexes are now better at deciding when to forecast
  (@lballabio); also added a `needsForecast` method that makes the
  information available.

- Added `CPI::laggedYoYRate`; also, `YoYInflationCoupon`,
  `yoyInflationLeg`, `CappedFlooredYoYInflationCoupon`,
  `YearOnYearInflationSwap`, `MakeYoYInflationCapFloor`,
  `YearOnYearInflationSwapHelper`, `YoYOptionletHelper` and the
  experimental `YoYCapFloorTermPriceSurface` and
  `InterpolatedYoYCapFloorTermPriceSurface` can now take an explicit
  `CPI::InterpolationType` parameter instead of relying on the index
  being defined as interpolated or not (@lballabio).  This is a first
  step in removing interpolation from `YoYInflationIndex` and moving
  it into the coupons where it belongs.

- Added method to YoY inflation index returning the date of the last
  available fixing (@lballabio).


Term structures
---------------

- Allow passing a pricer to the constructor of the `OISRateHelper` and
  `DatedOISRateHelper` classes (@lballabio); this makes it possible to
  use arithmetic averaging of overnight rates.

- Allow custom constraint in non-linear fitting methods; thanks to Kai
  Lin (@klin333).

- Allow creating a swap helper with frequency "Once" (@lballabio).

- The `GlobalBootstrap` constructor can now take an optional optimizer
  and end criteria, allowing for better configuration; thanks to
  Eugene Toder (@eltoder).


Volatility
----------

- Added exact Bachelier implied-vol formula from Jäckel's paper; thanks
  to Peter Caspers (@pcaspers).


Deprecated features
-------------------

- **Removed** features deprecated in version 1.31:
  - the `BlackVanillaOptionPricer` typedef;
  - the constructors of `CPICoupon` taking a `spread` parameter, its
    `spread` method, and its protected `spread_` data member;
  - the `withSpreads` method of `CPILeg`;
  - the protected `adjustedFixing` method and `spread_` data member of
    `CPICouponPricer`;
  - the `YYAUCPIr`, `YYEUHICPr`, `YYFRHICPr`, `YYUKRPIr`, `YYUSCPIr`
    and `YYZACPIr` indexes and the experimental `YYGenericCPIr` class;
  - the constructor of `YoYInflationIndex` taking a `ratio` parameter;
  - a couple of constructors of `ForwardRateAgreement`;
  - the empty files `ql/math/curve.hpp`, `ql/math/lexicographicalview.hpp`,
    `ql/termstructures/yield/drifttermstructure.hpp`
    and `ql/patterns/composite.hpp`;
  - the `const_iterator` and `const_value_iterator` typedefs in the
    `Garch11` class;
  - the `const_time_iterator`, `const_value_iterator`,
    `const_reverse_time_iterator` and `const_reverse_value_iterator`
    typedefs and the `cbegin_values`, `cend_values`, `crbegin_values`,
    `crend_values`, `cbegin_time`, `cend_time`, `crbegin_time` and
    `crend_time` methods of the `TimeSeries` class;
  - the `base`, `increment`, `decrement`, `advance` and `distance_to`
    method of the `step_iterator` class.

- Deprecated `ext::function`, `ext::bind`, `ext::ref`, `ext::cref`,
  `ext::placeholders`, `ext::tuple`, `ext::make_tuple`, `ext::get` and
  `ext::tie`; use the corresponding `std::` classes and functions
  instead.

- Deprecated the `ArithmeticAverageOIS`, `MakeArithmeticAverageOIS`
  and `ArithmeticOISRateHelper` classes; use `OvernightIndexedSwap`,
  `MakeOIS` and `OISRateHelper` instead.

- Deprecated the `YoYInflationCoupon`, `yoyInflationLeg`,
  `CappedFlooredYoYInflationCoupon`, `YearOnYearInflationSwap`,
  `MakeYoYInflationCapFloor`, `YearOnYearInflationSwapHelper`,
  `YoYOptionletHelper`, `YoYCapFloorTermPriceSurface` and
  `InterpolatedYoYCapFloorTermPriceSurface` constructors that don't
  take an explicit CPI interpolation type.

- Deprecated the `getInfo` method of `LevenbergMarquardt`; inspect the
  result of `minimize` instead.

- Deprecated the
  `ql/experimental/averageois/averageoiscouponpricer.hpp` file;
  include `ql/cashflows/overnightindexedcouponpricer.hpp` instead.

- Deprecated the somewhat out-of-scope and experimental
  `CreditRiskPlus`, `SensitivityAnalysis`, `aggregateNPV`,
  `parallelAnalysis` and `bucketAnalysis`.


**Thanks go also** to Jonathan Sweemer (@sweemer), Eugene Toder
 (@eltoder), Ralf Konrad Eckel (@ralfkonrad), Tony Wang (@twan3617)
 and the XAD team (@auto-differentiation-dev) for miscellaneous
 smaller fixes, improvements or reports.
