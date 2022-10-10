Changes for QuantLib 1.28:
==========================

QuantLib 1.28 includes 32 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/24?closed=1>.

Portability
-----------

- **New language standard:** as announced in the notes for the
  previous release, this release started using some C++14 syntax.
  This should be supported by most compilers released in the past
  several years.

- **End of support:** as announced in the notes for the previous
  release, this release is the last to manage thread-local singletons
  via a user-provided `sessionId` function.  Future releases will use
  the built-in language support for thread-local variables.

- **Future end of support:** after the next two or three releases,
  using `std::tuple`, `std::function` and `std::bind` (instead of
  their `boost` counterparts) will become the default.  If you're
  using `ext::tuple` etc. in your code (which is suggested), this
  should be a transparent change.  If not, you'll still be able to
  choose the `boost` versions via a configure switch for a while.

- A few non-standard configuration switches (`--enable-sessions`,
  `--enable-thread-safe-observer-pattern` and
  `--enable-thread-safe-singleton-init`) used to require the
  Boost.Thread library.  Thanks to Jonathan Sweemer (@sweemer), this
  is no longer the case.

Date/time
---------

- Added Act/366 and Act/365.25 day counters; thanks to Ignacio Anguita
  (@IgnacioAnguita).

- Added H.M. the Queen's funeral to the UK calendars; thanks to Tomass
  Wilson (@Wilsontomass).

Instruments
-----------

- Amortizing bonds were moved out of the experimental folder.  Also, a
  couple of utility functions were provided to calculate amortization
  schedules and notionals.

Pricing engines
---------------

- Fixed results from `COSHestonEngine` in the case of an option with
  short time to expiration and deep ITM or deep OTM strike prices;
  thanks to Ignacio Anguita (@IgnacioAnguita).

- The ISDA engine for CDS could calculate the fair upfront with the
  wrong sign; this is now fixed, thanks to Gualtiero Chiaia
  (@gchiaia).

Term structures
---------------

- The constructor for `OISRateHelper` now allows to specify the
  `endOfMonth` parameter; thanks to Guillaume Horel (@thrasibule).

Finite differences
------------------

- Fixed computation of cds boundaries in `LocalVolRNDCalculator`;
  thanks to @mdotlic.

Experimental folder
-------------------

The `ql/experimental` folder contains code whose interface is not
fully stable, but is released in order to get user
feedback. Experimental classes make no guarantees of backward
compatibility; their interfaces might change in future releases.

- **Breaking change**: the constructor of the
  `CPICapFloorTermPriceSurface` class now also takes an explicit
  interpolation type.

- **Possibly breaking**: the protected constructor for `CallableBond`
  changes its arguments.  If you inherited from this class, you'll
  need to update your code.  If you're using the existing derived bond
  classes, the change will be transparent.

- Pricing engines for callable bonds worked incorrectly when the face
  amount was not 100. This is now fixed.

- The `impliedVolatility` method for callable bonds was taking a
  target NPV, not a price. This implementation is now deprecated, and
  a new overload was added taking a price in base 100.

Deprecated features
-------------------

- **Removed** features deprecated in version 1.23:
  - the constructors of `ZeroCouponInflationSwap` and
    `ZeroCouponInflationSwapHelper` missing an explicit CPI
    interpolation type;
  - the constructors of `ActualActual` and `Thirty360` missing an
    explicit choice of convention, and the constructor of `Thirty360`
    passing an `isLastPeriod` boolean flag.

- Deprecated the constructors of `FixedRateBond` taking an
  `InterestRate` instance or not taking a `Schedule` instance.

- Deprecated the constructor of `FloatingRateBond` not taking a
  `Schedule` instance.

- Deprecated the constructors of `AmortizingFixedRateBond` taking a
  sinking frequency or a vector of `InterestRate` instances.

- Deprecated the constructor of `CPICapFloor` taking a `Handle` to an
  inflation index, and its `inflationIndex` method returning a `Handle`.
  New versions of both were added using `shared_ptr` instead.

- Deprecated one of the constructors of `SabrSmileSection`; a new
  version was added also taking an optional reference date.

- Deprecated the old `impliedVolatility` method for callable bonds;
  see above.


**Thanks go also** to Konstantin Novitsky (@novitk), Peter Caspers
(@pcaspers), Klaus Spanderen (@klausspanderen) and Fredrik Gerdin
Börjesson (@gbfredrik) for a number of smaller fixes, and to Jonathan
Sweemer (@sweemer) for various improvements to the automated CI
builds.

