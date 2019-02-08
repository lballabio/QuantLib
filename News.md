
Changes for QuantLib 1.15:
==========================

QuantLib 1.15 includes 32 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/11?closed=1>.

Portability
-----------

- This release drops support for Boost version 1.43 to 1.47; the
  minimum required version is now Boost 1.48, released in 2011.

- Added a `.clang-format` file to the repository.  The format is not
  going to be enforced, but the style file is provided as a
  convenience in case you want to format new code according to the
  conventions of the library.

- `boost::function`, `boost::bind` and a few related classes and
  functions were imported into the new namespace `QuantLib::ext`.
  This allows them to be conditionally replaced with their `std::`
  versions (see the "opt-in features" section below).  The default is
  still to use the Boost implementation.  Client code using the
  `boost` namespace explicitly doesn't need to be updated.

Models
------

- Added an experimental volatility basis model for caplet and swaptions
  (thanks to Sebastian Schlenkrich).

Pricing engines
---------------

- It is now possible to specify polynomial order and type when
  creating a `MCAmericanBasketEngine` instance (thanks to Klaus
  Spanderen).

Term structures
---------------

- Inflation curves used to store the nominal curve used during their
  construction.  This is still supported for backward compatibility,
  but is deprecated.  You should instead pass the nominal curve
  explicitly to objects that need one (e.g., inflation helpers,
  engines, or cashflow pricers).

- Added experimental helpers to bootstrap an interest-rate curve on
  SOFR futures (thanks to Roy Zywina).

Indexes
-------

- It is now possible to choose the fixing calendar for the BMA index
  (thanks to Jan Ladislav Dussek).

Cash flows
----------

- Fixed broken observability in CMS-spread coupon pricer (thanks to
  Peter Caspers).

Date/time
---------

- Fix implementation of Actual/Actual (ISMA) day counter in case a
  schedule is provided (thanks to Philip Stephens).

- Fix implementation of `Calendar::businessDaysBetween` method when
  the initial and final date are the same (thanks to Weston Steimel).

- Added day of mourning for G.H.W. Bush to the list of United States
  holidays (thanks to Joshua Engelman).

- Updated list of Chinese holidays for 2019 (thanks to Cheng Li).

- Added basic unit tests for the `TimeGrid` class (thanks to Kai
  Striega).

Math
----

- Prevent solver failure in Richardson extrapolation (thanks to Klaus
  Spanderen).

Examples
--------

- Added multi-curve bootstrapping example (thanks to Jose
  Garcia). This examples supersedes the old swap-valuation example,
  that was therefore removed.

Deprecated features
-------------------

- Up to this release, it has been possible to force interest rates to
  be non-negative by commenting the `QL_NEGATIVE_RATES` macro in
  `ql/userconfig.hpp` on Visual C++ or by passing the
  `--disable-negative-rates` switch to `./configure` on other systems.
  This possibility will no longer be supported in future releases.

New opt-in features
-------------------

- It is now possible to use `std::function`, `std::bind` and their
  related classes instead of `boost::function` and `boost::bind`.  The
  feature can be enabled by uncommenting the `QL_USE_STD_FUNCTION`
  macro in `ql/userconfig.hpp` on Visual C++ or by passing the
  `--enable-std-function` switch to `./configure` on other systems.
  This requires using at least the C++11 standard during compilation.

- A new `./configure` switch, `--enable-std-classes`, was added as a
  shortcut for `--enable-std-pointers` `--enable-std-unique-ptr`
  `--enable-std-function`.
