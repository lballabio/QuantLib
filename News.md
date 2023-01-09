Changes for QuantLib 1.29:
==========================

QuantLib 1.29 includes 42 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/26?closed=1>.


Portability
-----------

- **End of support:** as announced in the notes for the previous
  release, this release no longer manages thread-local singletons via
  a user-provided `sessionId` function, and therefore the latter is no
  longer needed.  Instead, the code now uses the built-in language
  support for thread-local variables.  Thanks go to Peter Caspers
  (@pcaspers).

- **Future end of support:** as announced in the notes for the
  previous release, after the next couple of releases, using
  `std::tuple`, `std::function` and `std::bind` (instead of their
  `boost` counterparts) will become the default.  If you're using
  `ext::tuple` etc. in your code (which is suggested), this should be
  a transparent change.  If not, you'll still be able to choose the
  `boost` versions via a configure switch for a while; but we do
  suggest you start using `ext::tuple` etc. in the meantime.

- Replaced internal usage of `boost::thread` with `std::thread`;
  thanks to Jonathan Sweemer (@sweemer).  This removed our last
  dependency on Boost binaries and makes it possible to compile
  QuantLib using a header-only Boost installation.

- On Windows, it is now possible to use the MSVC dynamic runtime when
  using cmake by passing
  `-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`
  on the command line; thanks to Jonathan Sweemer (@sweemer).  The
  static runtime remains the default.

- It is now possible to build QuantLib with Intel's `icpx` compiler
  using cmake; thanks to Jonathan Sweemer (@sweemer).  Note that in
  order to get all the unit tests passing, `-fp-model=precise` must be
  added to `CMAKE_CXX_FLAGS`.


Date/time
---------

- Updated Chinese holidays for 2023; thanks to Cheng Li
  (@wegamekinglc).

- Added in-lieu holiday for Christmas 2022 to South-African calendar;
  thanks to Joshua Hayes (@JoshHayes).

- Added King Charles III coronation holiday to UK calendar; thanks to
  Fredrik Gerdin Börjesson (@gbfredrik).

- Added holiday for National Day of Mourning to Australian calendar;
  thanks to Fredrik Gerdin Börjesson (@gbfredrik).


Instruments
-----------

- Added high performance/precision American engine based on
  fixed-point iteration for the exercise boundary; thanks to Klaus
  Spanderen (@klausspanderen).

- Bonds with draw-down (i.e., increasing notionals) are now allowed;
  thanks to Oleg Kulkov (@Borgomi42 ).

- Added `withIndexedCoupons` and `withAtParCoupons` methods to
  `MakeSwaption` for easier initialization; thanks to Ralf Konrad
  (@ralfkonrad).

- It is now possible to use the same pricing engine for vanilla and
  dividend vanilla options, or for barrier and dividend barrier
  options (@lballabio).


Indexes
-------

- Creating a zero inflation index as "interpolated" is now deprecated;
  thanks to Ralf Konrad (@ralfkonrad).  The index should only return
  monthly fixings.  Interpolation is now the responsibility of
  inflation-based coupons.


Term structures
---------------

- The `ConstantCPIVolatility` constructor can now take a handle to a
  volatility quote, instead of just an immutable number (@lballabio).


Deprecated features
-------------------

- **Removed** features deprecated in version 1.24:
  - the `createAtParCoupons`, `createIndexedCoupons` and
    `usingAtParCoupons` methods of `IborCoupon`;
  - the `RiskyBond` class and its subclasses `RiskyFixedBond` and
    `RiskyFloatingBond`;
  - the `CrossCurrencyBasisSwapRateHelper` typedef;
  - the `termStructure_` data member of `BlackCalibrationHelper`;
  - the static `baseCurrency` and `conversionType` data members of `Money`;
  - the `nominalTermStructure` method and the `nominalTermStructure_`
    data member of `InflationTermStructure`;
  - the constructor of the `UnitedStates` calendar not taking an
    explicit market.

- Deprecated the `argument_type`, `first_argument_type`,
  `second_argument_type` and `result_type` typedefs in a number of
  classes; use `auto` or `decltype` instead.

- Deprecated the constructors of `InflationIndex`,
  `ZeroInflationIndex`, `FRHICP`, `ZACPI`, `UKRPI`, `EUHICP`,
  `EUHICPXT`, `USCPI`, `AUCPI` and `GenericCPI` taking an
  `interpolated` parameter; use another constructor.

- Deprecated the `interpolated` method and the `interpolated_` data
  member of `InflationIndex`.

- Deprecated the `ThreadKey` typedef.  It was used in the signature of
  `sessionId`, which is no longer needed after the changes in the
  `Singleton` implementation.

- Deprecated the `rateCurve_` data member of the
  `InflationCouponPricer` base class.  If you need it, provide it in
  your derived class.

- Deprecated the `npvbps` function taking NPV and BPS as references.
  Use the overload returning a pair of `Real`s.


**Thanks go also** to Matthias Groncki (@mgroncki), Jonathan Sweemer
(@sweemer) and Nijaz Kovacevic (@NijazK) for a number of smaller fixes
and improvements, to the Xcelerit Dev Team (@xcelerit-dev) for
improvements to the automated CI builds, and to Vincenzo Ferrazzanno
(@vincferr), @alienbrett, @xuruilong100 and @philippb90 for raising issues.
