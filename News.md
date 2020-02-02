Changes for QuantLib 1.17:
==========================

QuantLib 1.17 includes 30 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/13?closed=1>.

Portability
-----------

- As of this release, support of Visual C++ 2010 is deprecated; it
  will be dropped in next release.  Also, we'll probably deprecate
  Visual C++ 2012 in one of the next few releases in order to drop it
  around the end of 2020.

Configuration
-------------

- A new function `compiledBoostVersion()` is available, (thanks to
  Andrew Smith).  It returns the version of Boost used to compile the
  library, as reported by the `BOOST_VERSION` macro.  This can help
  avoid linking the library with user code compiled with a different
  Boost version (which can result in erratic behavior).

- It is now possible to specify at run time whether to use indexed
  coupons (thanks to Ralf Konrad).  The compile-time configuration is
  still used as a default, but it is also possible to call either of
  the static methods `IborCoupon::createAtParCoupons` or
  `IborCoupon::createIndexedCoupons` to specify your preference.  For
  the time being, the methods above must necessarily be called before
  creating any instance of `IborCoupon` or of its derived classes.

Build
-----

- As of this version, the names of the binaries produced by the
  included Visual C++ solution no longer contain the toolset version
  (e.g., v142).

Instruments
-----------

- Added ex-coupon functionality to floating-rate bonds (thanks to
  Steven Van Haren).

- The inner structure `Callability::Price` was moved to the class
  `Bond` and can now be used to specify what kind of price was passed
  to the `BondFunctions::yield` method (thanks to Francois Botha).

- It is now possible to use a par-coupon approximation for FRAs like
  the one used in Ibor coupons (thanks to Peter Caspers).

Pricing engines
---------------

- Added escrowed dividend model to the new-style FD engine for
  `DividendVanillaOption` (thanks to Klaus Spanderen).

- Black cap/floor engine now also returns caplet deltas (thanks to
  Wojciech Slusarski).

Term structures
---------------

- OIS rate helpers can now choose whether to use as a pillar for the
  bootstrap either their maturity date or the end date of the last
  underlying fixing.  This provides an alternative if the bootstrap
  should fail.  (Thanks to Drew Saunders for the heads-up.)

- Instances of the `FittedBondDiscountCurve` class now behave as
  simple evaluators (that is, they use the given paramters without
  performing root-solving) when the `maxIterations` parameter is set
  to 0.  (Thanks to Nick Firoozye for the heads-up.)

Date/time
---------

- Added a few special closing days to the US government bond calendar
  (thanks to Mike DelMedico).

- Fixed an incorrect 2019 holiday in Chinese calendar (thanks to Cheng Li).

- Added missing holiday to Swedish calendar (thanks to GitHub users
  `periculus` and `tonyzhipengzhou`).

Deprecated features
-------------------

- The classes `FDEuropeanEngine`, `FDAmericanEngine`,
  `FDBermudanEngine`, `FDDividendEuropeanEngine`,
  `FDDividendEuropeanEngineShiftScale`, `FDDividendAmericanEngine`,
  `FDDividendAmericanEngineShiftScale` are now deprecated.  They are
  superseded by `FdBlackScholesVanillaEngine`.


Thanks go also to Joel King, Kai Striega, Francis Duffy, Tom Anderson
and GitHub user `lab4quant` for smaller fixes, enhancements, and bug
reports.
