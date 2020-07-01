Changes for QuantLib 1.19:
==========================

QuantLib 1.19 includes 40 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/15?closed=1>.

Portability
-----------

- Support for Visual C++ 2012 is being deprecated.  It will be dropped
  around the end of 2020 or the beginning of 2021 in order to enable
  use of C++11 features.

- Avoided use in Makefiles of functions only available to GNU Make
  (thanks to GitHub user `UnitedMarsupials` for the heads-up).

Build
-----

- Automated builds on Travis and GitHub Actions were extended.  We now
  have a build for Mac OS X, as well as a few builds that run a number
  of checks on the code (including clang-tidy) and automatically open
  pull requests with fixes.

Term structures
---------------

- Added options for iterative bootstrap to widen the search domain or
  to keep the best result upon failure (thanks to Francis Duffy).

- Added flat-extrapolation option to fitted bond curves (thanks to
  Peter Caspers).

Instruments
-----------

- Added finite-difference pricing engine for equity options under the
  Cox-Ingersoll-Ross process (thanks to Lew Wei Hao).

- Added Heston engine based on exponentially-fitted Laguerre
  quadrature rule (thanks to Klaus Spanderen).

- Added Monte Carlo pricing engines for lookback options (thanks to
  Lew Wei Hao).

- Added Monte Carlo pricing engine for double-barrier options (thanks
  to Lew Wei Hao).

- Added analytic pricing engine for equity options under the
  Vasicek model (thanks to Lew Wei Hao).

- The `Bond::yield` method can now specify a guess and whether the
  passed price is clean or dirty (thanks to Francois Botha).

Models
------

- Improved grid scaling for FDM Heston SLV calibration, and fixed
  drift and diffusion for Heston SLV process (thanks to Klaus
  Spanderen and Peter Caspers).

- Added mixing factor to Heston SLV process (thanks to Lew Wei Hao).

Math
----

- Improved nodes/weights for the exponentially fitted Laguerre
  quadrature rule and added sine and cosine quadratures (thanks to
  Klaus Spanderen).

Date/time
---------

- Improved performance of the Calendar class (thanks to Leonardo Arcari).

- Updated holidays for Indian and Russian calendars (thanks to Alexey
  Indiryakov).

- Added missing All Souls Day holiday to Mexican calendar (thanks to
  GitHub user `phil-zxx` for the heads-up).

- Restored New Year's Eve holiday to Eurex calendar (thanks to Joshua
  Engelman).

Deprecated features
-------------------

- Features deprecate in version 1.15 were removed: constructors of
  inflation swap helpers, inflation-based pricing engines and
  inflation coupon pricers that didn't take a nominal term structure.

- The constructor of `BMAIndex` taking a calendar was deprecated.

- The constructors of several interest-rate term structures taking
  jumps without a reference date were deprecated.

- The `CurveDependentStepCondition` class and related typedefs were
  deprecated.

- The constructor of `BlackCalibrationHelper` taking an interest-rate
  structure was deprecated.

- The constructors of several inflation curves taking a nominal curve
  were deprecated.  The nominal curve should now be passed to the used
  coupon pricers.

