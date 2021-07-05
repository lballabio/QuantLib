Changes for QuantLib 1.23:
==========================

QuantLib 1.23 includes 30 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/19?closed=1>.

Portability
-----------

- On Mac OS, the `-std=c++11` flag is now added automatically when
  needed.  This applies to both `configure` and `cmake` (thanks to
  Leander Schulten).

- We now assume that the compiler supports Boost::uBLAS and no longer
  check for it in configure.  (The check was originally introduced for
  versions of gcc before 4.x, which don't support C++ anyway.)
  Please let us know if this causes problems on some systems.

- The `Period`, `InterestRate` and `InterestRateIndex` classes are now
  visualized more clearly in the Visual Studio debugger (thanks to
  Francois Botha).

Cashflows
---------

- Year-on-year and CPI legs are now set a default coupon pricer.  In
  most cases, this removes the need for setting it explicitly.

- Add new `ZeroInflationCashFlow` class, used in zero-coupon inflation
  swaps (thanks to Ralf Konrad).

Currencies
----------

- Added custom constructor that allows to create bespoke currencies
  not already included in the library (thanks to Marcin Rybacki).

Date/time
---------

- Fixed implementation of U.S. 30/360 convention (the old one is still
  available as 30/360 NASD).

- The 30/360 ISDA convention can now take the termination date as a
  constructor argument and use it to adjust the calculation properly.

- Added the 30/360 ISMA convention; the Bond-Basis convention is now
  an alias to the former.

- The 30/360 German convention was renamed to ISDA; "German" remains
  as an alias.

- Added new Canadian holiday (National Day for Truth and
  Reconciliation) established in 2021 (thanks to GitHub user `qiubill`
  for the heads-up).

- Added new U.S. holiday (Juneteenth) established in 2021.

- Added new Platinum Jubilee U.K. holiday for 2022 (thanks to Ioannis
  Rigopoulos for the heads-up.)

- Added missing Christmas Eve holiday to Norwegian calendar (thanks to
  Prince Nanda).

Indexes
-------

- Added ESTR index (thanks to Magnus Mencke).

Instruments
-----------

- Added zero-coupon swap (thanks to Marcin Rybacki).

- The `Type` enumeration defined in several swap classes was moved to
  their base `Swap` class.

- Fixed sign of theta in experimental Kirk engine for spread options
  (thanks to Xu Ruilong for the heads-up).

Processes
---------

- Improved discretization of Cox-Ingersoll-Ross process to avoid
  occasional divergence (thanks to Magnus Mencke).

Deprecated features
-------------------

- Deprecated default constructor for actual/actual and 30/360 day
  counters; the desired convention should now be passed explicitly.

- Removed features deprecated in version 1.18: the
  `CalibrationHelperBase` typedef (now `CalibrationHelper`), some
  overloads of the `CalibratedModel::calibrate` and
  `CalibratedModel::value` methods, the constructors of
  `PiecewiseYieldCurve` and `PiecewiseDefaultCurve` taking an
  `accuracy` parameter, the constructors of `BondHelper`,
  `FixedRateBondHelper` and `CPIBondHelper` taking a boolean
  `useCleanPrice` parameter, the `BondHelper::useCleanPrice()` method,
  and the non-static `Calendar::holidayList` method.


Thanks go also to Francis Duffy, Kevin Kirchhoff, Magnus Mencke and
Klaus Spanderen for smaller fixes, enhancements and bug reports.
