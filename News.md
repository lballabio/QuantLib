Changes for QuantLib 1.18:
==========================

QuantLib 1.18 includes 34 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/14?closed=1>.

Portability
-----------

- As announced in the past release, support of Visual C++ 2010 is
  dropped.  Also, we'll probably deprecate Visual C++ 2012 in the next
  release in order to drop it around the end of 2020.

Build
-----

- Cmake now installs headers with the correct folder hierarchy (thanks
  to Cheng Li).

- The `--enable-unity-build` flag passed to configure now also causes
  the test suite to be built as a single source file.

- The Visual Studio projects now allow enabling unity builds as
  described at
  <https://devblogs.microsoft.com/cppblog/support-for-unity-jumbo-files-in-visual-studio-2017-15-8-experimental/>

Term structures
---------------

- A new `GlobalBootstrap` class can now be used with
  `PiecewiseYieldCurve` and other bootstrapped curves (thanks to Peter
  Caspers).  It allows to produce curves close to Bloomberg's.

- The experimental `SofrFutureRateHelper` class and its parent
  `OvernightIndexFutureRateHelper` can now choose to use either
  compounding or averaging, in order to accommodate different
  conventions for 1M and 3M SOFR futures (thanks to GitHub user
  `tani3010`).

- The `FraRateHelper` class has new constructors that take IMM start /
  end offsets (thanks to Peter Caspers).

- It is now possible to pass explicit minimum and maximum values to
  the `IterativeBootstrap` class.  The accuracy parameter was also
  moved to the same class; passing it to the curve constructor is now
  deprecated.

Instruments
-----------

- It is now possible to build fixed-rate bonds with an arbitrary
  schedule, even without a regular tenor (thanks to Steven Van Haren).

Models
------

- It is now possible to use normal volatilities to calibrate a
  short-rate model over caps.

Date/time
---------

- The Austrian calendar was added (thanks to Benjamin Schwendinger).

- The German calendar incorrectly listed December 31st as a holiday;
  this is now fixed (thanks to Prasad Somwanshi).

- Chinese holidays were updated for 2020 and the coronavirus event
  (thanks to Cheng Li).

- South Korea holidays were updated for 2016-2020 (thanks to GitHub
  user `fayce66`).

- In the calendar class, `holidayList` is now an instance method; the
  static version is deprecated.  The `businessDayList` method was also
  added.  (Thanks to Piotr Siejda.)

- A bug in the 30/360 German day counter was fixed (thanks to Kobe
  Young for the heads-up).

Optimizers
----------

- The differential evolution optimizer was updated (thanks to Peter
  Caspers).

Currencies
----------

- Added Kazakstani Tenge to currencies (thanks to Jonathan Barber).

Deprecated features
-------------------

- Features deprecate in version 1.14 were removed: one of the
  constructors of the `BSMOperator` class, the whole `OperatorFactory`
  class, and the typedef `CalibrationHelper` which was used to alias
  the `BlackCalibrationHelper` class.

- The `CalibrationHelperBase` class is now called
  `CalibrationHelper`. The old name remains as a typedef but is
  deprecated.

- The overload of `CalibratedModel::calibrate` and
  `CalibratedModel::value` taking a vector of
  `BlackCalibrationHelper`s are deprecated in favor of the ones taking
  a vector of `CalibrationHelper`s.

- The static method `Calendar::holidayList` is deprecated in favor of
  the instance method by the same name.

- The constructors of `PiecewiseDefaultCurve` and
  `PiecewiseYieldCurve` taking an accuracy parameter are deprecated in
  favor of passing the parameter to an instance of the bootstrap
  class.

- The constructors of `BondHelper` and derived classes taking a
  boolean flag to choose between clean and dirty price are deprecated
  in favor of the ones taking a `Bond::Price::Type` argument.  The
  `useCleanPrice` method is also deprecated in favor of `priceType`.


Thanks go also to Ralf Konrad, Klaus Spanderen, Carlos Fidel Selva
Ochoa, F. Eugene Aumson and Francois Botha for smaller fixes,
enhancements, and bug reports.
