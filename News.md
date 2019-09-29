
Changes for QuantLib 1.16:
==========================

QuantLib 1.16 includes 34 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/12?closed=1>.

Portability
-----------

- Added support for Visual Studio 2019 (thanks to Paul Giltinan).

Configuration
-------------

- As announced in past release, the compile-time switch to force
  non-negative rates was removed.

Pricing engines
---------------

- Added constant elasticity of variance (CEV) pricing engines for
  vanilla options.  Analytic, FD and SABR engines are available
  (thanks to Klaus Spanderen).

- Added quanto pricing functionality to a couple of FD engines for
  DividendVanillaOption (thanks to Klaus Spanderen).

Cash flows
----------

- Digital coupons can now optionally return the value of the naked
  option (thanks to Peter Caspers).

Date/time
---------

- Updated Taiwan holidays for 2019 (thanks to Hank Liu).

- Added two newly announced holidays to Chinese calendar (thanks to
  Cheng Li).

- Updated Japan calendar (thanks to Eisuke Tani).

- Fixed New Year's day adjustment for Canadian calendar (thanks to Roy
  Zywina).

- Added a couple of exceptions for UK bank holidays (thanks to GitHub
  user Vililikku for the heads-up).

- Added French calendar (thanks to GitHub user NJeanray).

- Added public methods to expose a calendar's added and removed
  holidays (thanks to Francois Botha).

- Allow the stub date of a schedule to equal the maturity.


Deprecated features
-------------------

- Deprecated a constructor of the SwaptionVolatilityMatrix class that
  didn't take a calendar.

- Removed typedefs GammaDistribution, ChiSquareDistribution,
  NonCentralChiSquareDistribution and
  InverseNonCentralChiSquareDistribution, deprecated in version 1.12.
  Use CumulativeGammaDistribution, CumulativeChiSquareDistribution,
  NonCentralCumulativeChiSquareDistribution and
  InverseNonCentralCumulativeChiSquareDistribution instead.

- Removed Actual365NoLeap class, deprecated in version 1.11.  It was
  folded into Actual365Fixed.

Term structures
---------------

- Take payment days into account when calculating the nodes of a
  bootstrapped curve based on overnight swaps.
