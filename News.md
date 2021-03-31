Changes for QuantLib 1.22:
==========================

QuantLib 1.22 includes 53 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/18?closed=1>.

Portability
-----------

- As previously announced, this release drops support for Visual
  C++ 2012.  VC++ 2013 or later is now required.

- The `Date` and `Array` classes are now visualized more clearly in
  the Visual Studio debugger (thanks to Francois Botha).

Language standard
-----------------

- QuantLib now uses the C++11 standard and no longer compiles in C++03
  mode.  As before, it can be compiled with later versions of the
  standard.  For details on the C++11 features used, see the pull
  requests marked "C++11 modernization" at the above link; for
  information on possible problems, see
  <https://www.implementingquantlib.com/2021/02/leaving-03-for-real.html>.

Cashflows
---------

- Revised and tested the `SubPeriodCoupon` class (thanks to Marcin
  Rybacki).  The class was moved out of the `ql/experimental` folder
  and its interface can now be considered stable.

- Add simple averaging to overnight-index coupons in addition to the
  existing compound averaging (thanks to Marcin Rybacki).

- Fixed accrual calculation for inflation coupon when trading
  ex-coupon (thanks to GitHub user `bachhani`).

Currencies
----------

- Added the Nigerian Naira (thanks to Bryte Morio).

Date/time
---------

- Fixed actual/actual (ISMA) day counter calculation for long/short
  final periods (thanks to Francois Botha).

- Updated a couple of changed rules for New Zealand calendar (thanks
  to Paul Giltinan).

Indexes
-------

- Added `hasHistoricalFixing` inspector to `Index` class to check if
  the fixing for a given past date is available (thanks to Ralf
  Konrad).

Instruments
-----------

- Added new-style finite-difference engine for shout options (thanks
  to Klaus Spanderen).  In the case of dividend shout options, an
  escrowed dividend model is used.

- Revised the `OvernightIndexFutures` class.  The class was moved out
  of the `ql/experimental` folder and its interface can now be
  considered stable.

- Added an overloaded constructor for Asian options that takes all
  past fixings and thus allows to reprice them correctly when the
  evaluation date changes (thanks to Jack Gillett).

- Added support for seasoned geometric Asian options to the Heston
  engine (thanks to Jack Gillett).

Patterns
--------

- Faster implementation of the `Observable` class in the thread-safe
  case (thanks to Klaus Spanderen).

Term structures
---------------

- Added experimental rate helper for constant-notional cross-currency
  basis swaps (thanks to Marcin Rybacki).

- Added volatility type and displacements to year-on-year inflation
  volatility surfaces (thanks to Peter Caspers).

Deprecated features
-------------------

- Removed features deprecated in version 1.17: the `Callability::Type`
  typedef (now `Bond::Price`), the `FdmOrnsteinUhlenbackOp` typedef
  (now correctly spelled as `FdmOrnsteinUhlenbeckOp`, and a number of
  old-style finite-difference engines (`FDAmericanEngine`,
  `FDBermudanEngine`, `FDDividendAmericanEngine` and its variants,
  `FDDividendEuropeanEngine` and its variants, and `FDEuropeanEngine`)
  all replaced by the `FdBlackScholesVanillaEngine` class.

- Deprecated the old-style finite difference engines for shout
  options; they are now replaced by the new `FDDividendShoutEngine`
  class.

- Deprecated a few unused parts of the old-style finite-differences
  framework: the `AmericanCondition` class, the `OneFactorOperator`
  typedef, and the `FDAmericanCondition` class.

Test suite
----------

- Reduced the run time for the longest-running test cases.

Thanks go also to Francis Duffy and Cay Oest for smaller fixes,
enhancements and bug reports.
