Changes for QuantLib 1.38:
==========================

QuantLib 1.38 includes 29 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/36?closed=1>.


Portability
-----------

- **Future change of default:** as already announced, in the next
  release we're going to switch the default for `ext::any` and
  `ext::optional` from the Boost implementation to the standard one.
  Using `boost::any` and `boost::optional` is still possible
  for the time being but deprecated.

- **Possible future breaking change**: in the next release, the
  `SimpleQuote` class might be made `final`.  If you're inheriting
  from it, drop us a line.


Dates and calendars
-------------------

- The `Schedule` class now honors the passed business day convention
  when end-of-month is enabled (@lballabio).  Previously, enabling
  end-of-month caused it to always use the Modified Following
  convention.

- Added Chinese holidays for 2025; thanks to Cheng Li (@wegamekinglc).

- Added Thailand holidays for 2025; thanks to Paolo D'Elia
  (@paolodelia99).

- Added Hong Kong holidays for 2025; thanks to Ka Wai Lee (@kawailee).


Indexes
-------

- Year-or-year inflation indexes can (and should) now be built without
  an `interpolated` flag (@lballabio).  As for zero inflation indexes,
  the interpolation was moved into the coupons using the indexes.

- Fixed obsolete conventions for the (now discountinued) EUR LIBOR
  index; thanks to Eugene Toder (@eltoder).


Instruments and pricing engines
-------------------------------

- Added implementation of partial-time barrier put options; thanks to
  Paolo D'Elia (@paolodelia99).

- The `OvernightIndexFuture` class would not receive notifications
  when the convexity quote or the evaluation date changed; this is now
  fixed.  Thanks to Eugene Toder (@eltoder).

- The experimental `BlackCallableFixedRateBondEngine` wouldn't take
  discount correctly into account when evaluation the embedded option;
  this is now fixed.  Thanks to @RobertS548 for the heads-up.

- Moved a few instruments and engines from the experimental folder to
  the core library (@lballabio):
  - `HolderExtensibleOption` and `AnalyticHolderExtensibleOptionEngine`;
  - `WriterExtensibleOption` and `AnalyticWriterExtensibleOptionEngine`;
  - `PartialTimeBarrierOption` and `AnalyticPartialTimeBarrierOptionEngine`;
  - `TwoAssetBarrierOption` and `AnalyticTwoAssetBarrierEngine`;
  - `TwoAssetCorrelationOption` and ``AnalyticTwoAssetCorrelationEngine`;
  - `ContinuousArithmeticAsianLevyEngine`;
  - `AnalyticPDFHestonEngine`.


Term structures
---------------

- The `DepositRateHelper` and `FraRateHelper` classes can now be built
  specifying fixed dates instead of a tenor; thanks to Eugene Toder
  (@eltoder).

- The cross-currency basis-swap rate helpers can now be passed an
  overnight index and a corresponding payment frequency; it is also
  possible to pass a payment lag.  Thanks to @kp9991-git.

- The additional penalty functions passed to the `GlobalBootstrap`
  class can now take the curve nodes as arguments; thanks to Eugene
  Toder (@eltoder).  This makes it possible, for example, to penalize
  gradients to make the curve smoother.  It is also possible to
  specify additional variables to be optimized, e.g., futures
  convexity adjustments.

- Added a piecewise forward-spreaded term structure; thanks to
  Paolo D'Elia (@paolodelia99).


Deprecated features
-------------------

- **Removed** features deprecated in version 1.33:
  - the constructors of `Currency` and `Currency::Data` taking a
    format string, the `format` method of the `Currency` class and the
    `formatString` data member of `Currency::Data`.

- Deprecated the constructors of year-on-year inflation indexes taking
  an `interpolated` argument; use the other constructors instead.

- Deprecated the header files in `ql/experimental/exoticoptions` for
  some classes moved to the core library (see above); use the
  corresponding new headers in `ql/instruments` and
  `ql/pricingengines` instead.


**Thanks go also** to Eugene Toder (@eltoder), Konstantin Novitsky
(@novitk), Tomas Kalibera (@kalibera) and @raneamri for miscellaneous
smaller fixes, improvements or reports.
