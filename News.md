Changes for QuantLib 1.25:
==========================

QuantLib 1.25 includes 35 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/21?closed=1>.

Portability
-----------

- **End of support:** this release and the next will be the last two
  to support Visual Studio 2013.

- Added a few CMake presets for building the library (thanks to Jonathan Sweemer).

- When built and installed through CMake, the library now installs a `QuantLibConfig.cmake` file
  that allows other CMake projects to find and use QuantLib (thanks to Jonathan Sweemer).

Cashflows
---------

- Fixed the accrual calculation in overnight-indexed coupons (thanks to Mohammad Shojatalab).

- Fixed fixing-days usage in `SubPeriodsCoupon` class (thanks to Marcin Rybacki).

- IBOR coupons fixed in the past no longer need a forecast curve to return their amount.

Indexes
-------

- **Important change:** inflation indexes inherited from the `ZeroInflationIndex`
  class no longer rely on their forecast curve for interpolation.  For coupons
  that already took care of interpolation (as in the case of `CPICoupon` and
  `ZeroInflationCashFlow`) this should not change the results. In other cases,
  figures will change but should be more correct as the interpolation is now
  performed according to market conventions.
  Also, most inflation curves now assume that the index is not implemented.
  Year-on-year inflation indexes and curves are not affected.

Instruments
-----------

- **Breaking change:** convertible bonds were moved out of the `ql/experimental` folder.
  Also, being market values and not part of the contract, dividends and credit spread
  were moved from the bond to the `BinomialConvertibleEngine` class
  (thanks to Lew Wei Hao).

- The `ForwardRateAgreement` no longer inherits from `Forward`.  This also made it
  possible to implement the `amount` method returning the expected cash settlement
  (thanks to Lew Wei Hao).  The methods from `Forward` were kept available but
  deprecated so code using them won't break.  Client code might break if it
  performed casts to `Forward`.

Models
------

- Fixed formula for discount bond option in CIR++ model (thanks to Magnus Mencke).

Term structures
---------------

- It is now possible to use normal volatilities in SABR smile sections,
  and thus in the `SwaptionVolCube1` class (thanks to Lew Wei Hao).

Date/time
---------

- Added Chinese holidays for 2022 (thanks to Cheng Li).

Currencies
----------

- Added a number of African, American, Asian and European currencies from
  Quaternion's `QuantExt` project (thanks to Ole Bueker).

Experimental folder
-------------------

The `ql/experimental` folder contains code whose interface is not
fully stable, but is released in order to get user
feedback. Experimental classes make no guarantees of backward
compatibility; their interfaces might change in future releases.

- Added experimental rate helpers for LIBOR-LIBOR and Overnight-LIBOR basis swaps.

- Renamed `WulinYongDoubleBarrierEngine` to `SuoWangDoubleBarrierEngine`
 (thanks to Adityakumar Sinha for the fix and Ruilong Xu for the heads-up).

Deprecated features
-------------------

- Deprecated the constructors of zero-coupon inflation term structures taking
  an `indexIsInterpolated` boolean argument.

- Deprecated a number of methods in the `ForwardRateAgreement` class that used
  to be inherited from `Forward`.

- Deprecated a couple of constructors in the `SofrFutureRateHelper` class.

- Deprecated the `WulinYongDoubleBarrierEngine` alias for `SuoWangDoubleBarrierEngine`.

- Deprecated the protected `spreadLegValue_` data member
  in the `BlackIborCouponPricer` class.


Thanks go also to Tom Anderson, Francois Botha, Matthew Kolbe, Benson
Luk, Marcin Rybacki, Henning Segger, Klaus Spanderen, and GitHub users
@jxcv0 and @azsrz for smaller fixes, enhancements and bug reports.
