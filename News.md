Changes for QuantLib 1.39:
==========================

QuantLib 1.39 includes 28 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/37?closed=1>.


Portability
-----------

- **Bug in latest Visual C++ version**: at the time of this writing,
  the latest version of the Visual C++ 2022 compiler (the 17.14.7
  version, using the 14.44 toolset) has a known bug that,
  unfortunately, affects QuantLib heavily and makes it basically
  unusable.  A fix has been implemented and will be released at some
  point in the near future, but in the meantime, if you’re compiling
  QuantLib on Windows, either use the Visual C++ 2019 toolset (you can
  do that from VC++ 2022, as well) or use a less recent version that
  still uses the 14.43 toolset.

- **Change of default:** as already announced, in this release we're
  switching the default for `ext::any` and `ext::optional` from the
  Boost implementation to the standard one.  Using `boost::any` and
  `boost::optional` is still possible for the time being but
  deprecated.


Dates, calendars and day-count conventions
------------------------------------------

- Fixed a corner case of `Calendar::advance` when using EOM and the
  unadjusted business-day convention; thanks to Eugene Toder (@eltoder).
  
- Fixed an error when asking for the serial number of a null date with
  intraday support enabled (@lballabio); thanks to @UnitedMarsupial
  for the heads-up.

- Added the SHIR fixing calendar (@lballabio).

- Fixed the order of operations in the 30/360 USA day-count
  convention; thanks to Eugene Toder (@eltoder).


Indexes
-------

- Added the SARON index; thanks to Paolo D'Elia (@paolodelia99).

- Added a `CustomIborIndex` class that allows to create an IBOR-like
  index with custom calendars for value and maturity dates
  calculations; thanks to Eugene Toder (@eltoder).


Instruments and pricing engines
-------------------------------

- The `MakeOIS` class now knows the default number of settlement days
  for a few currencies; thanks to Zak Kraehling (@7astro7).


Interest rates
--------------

- The `FxSwapRateHelper` class can now be built specifying fixed dates
  instead of a tenor; thanks to Eugene Toder (@eltoder).

- A number of helpers can now take quoted rates either as numbers or
  `Handle<Quote>` via the use of `std::variant`; this reduces the
  number of overloaded constructors and in some cases allows the use
  of keyword arguments when exported to Python.  Thanks to Paolo
  D'Elia (@paolodelia99) and Eugene Toder (@eltoder).

- The `OISRateHelper` class can now specify a calendar for the
  overnight leg; thanks to Eugene Toder (@eltoder).
  
- The `ZeroCouponInflationSwapHelper` class now doesn't need to be
  passed a nominal curve, which wouldn't affect the results anyway
  (@lballabio).


Volatility
----------

- Optionlet stripperes can now use overnight indexes; thanks to Paolo
  D'Elia (@paolodelia99).
  
- Added calculation of better guesses for SABR calibration as detailed
  in the Le Floc'h and Kennedy paper (@lballabio).


Deprecated features
-------------------

- **Removed** features deprecated in version 1.34:
  - the overloads of `Bond::yield`, `BondFunctions::atmRate`,
    `BondFunctions::yield` and `BondFunctions::zSpread` taking a price
    as a `Real` instead of a `Bond::Price` instance;
  - the `Swaption::underlyingSwap` and
    `SwaptionHelper::underlyingSwap` methods;
  - the constructors of `InflationTermStructure`,
    `ZeroInflationTermStructure`, `YoYInflationTermStructure`,
    `InterpolatedZeroInflationCurve`, `InterpolatedYoYInflationCurve`,
    `PiecewiseZeroInflationCurve` and `PiecewiseYoYInflationCurve`
    taking an observation lag;
  - the overload of `InflationTermStructure::setSeasonality` taking no arguments;
  - the `InflationTermStructure::setBaseRate` method;
  - the `fixedRateBond` method and `fixedRateBond_` data member of the
    `FixedRateBondHelper` class, and the `cpiBond` method and
    `cpiBond_` data member of the `CPIBondHelper` class.

- Deprecated the `observationLag` and `hasExplicitBaseDate` methods
  and the `observationLag_` data member of the
  `InflationTermStructure` class; inflation term structures always
  have an explicit base date now.

- Deprecated the usage of `boost::any` and `boost::optional`; their
  standard counterparts are used by default now.

- Deprecated the constructor of `ZeroCouponInflationSwapHelper` taking
  a nominal curve; use the other constructor instead.


**Thanks go also** to Imrane Amri (@raneamri), Ralf Konrad Eckel
(@ralfkonrad), Joan Carlos Naftanaila (@MiDDiz), Eugene Toder
(@eltoder), Paolo D'Elia (@paolodelia99) and Holger Rother (@hrother)
for miscellaneous smaller fixes, improvements or reports.
