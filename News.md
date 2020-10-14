Changes for QuantLib 1.20:
==========================

QuantLib 1.20 includes 24 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/16?closed=1>.

Portability
-----------

- Support for Visual C++ 2012 is being deprecated.  It will be dropped
  after the next release in order to enable use of C++11 features.

- It is now possible to opt into using `std::tuple` instead of
  `boost::tuple` when the compiler allows it.  The default is still to
  use the Boost implementation.  The feature can be enabled by
  uncommenting the `QL_USE_STD_TUPLE` macro in `ql/userconfig.hpp` on
  Visual C++ or by passing the `--enable-std-tuple` switch to
  `./configure` on other systems.  The `--enable-std-tuple` switch is
  also implied by `--enable-std-classes`.  (Thanks to Joseph Wang.)

Instruments
-----------

- Added mixing-factor parameter to Heston finite-differences barrier,
  rebate and double-barrier engines (thanks to Jack Gillett).

- Added a few additional results to Black swaption engine and to
  analytic European option engine (thanks to Peter Caspers and Marcin
  Rybacki).

- Improved calculation of spot date for vanilla swap around holidays
  (thanks to Paul Giltinan).

- Added ex-coupon feature to amortizing bonds, callable bonds and
  convertible bonds.

- Added optional first-coupon day counter to fixed-rate bonds (thanks
  to Jacob Lee-Howes).

Math
----

- Added convenience classes `LogCubic` and `LogMixedLinearCubic`
  hiding a few default parameters (thanks to Andrea Maffezzoli).

Models
------

- Added control variate based on asymptotic expansion for the Heston
  model (thanks to Klaus Spanderen).

Date/time
---------

- Added missing Hong Kong holiday (thanks to GitHub user `CarrieMY`).

- Added a couple of one-off closing days to the Romanian calendar.

- Added a one-off holiday to South Korean calendar (thanks to GitHub
  user `fayce66`).

- Added a missing holiday to Turkish calendar (thanks to Berat
  Postalcioglu).

Documentation
-------------

- Added basic documentation to optimization methods (thanks to GitHub
  user `martinbrose`).

Deprecated features
-------------------

- Features deprecate in version 1.16 were removed: a constructor of
  the `FdmOrnsteinUhlenbeckOp` class and a constructor of the
  `SwaptionVolatilityMatrix` class.
