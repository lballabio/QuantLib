Changes for QuantLib 1.40
=========================

Removals and deprecations
-------------------------

Features deprecated in release 1.35 were removed in this release; see
<https://github.com/lballabio/QuantLib/pull/2268> for a full list.

A number of features were deprecated in this release and will be
removed in a future release:

- The overload of the `RangeAccrualFloatersCoupon` constructor taking
  a `shared_ptr` to a schedule; use the other overload instead.
- The method `observationsSchedule` of the same class; use
  `observationSchedule` instead.
- The `<ql/experimental/fx/blackdeltacalculator.hpp>` and
  `ql/experimental/fx/deltavolquote.hpp` headers; use
  `<ql/pricingengines/blackdeltacalculator.hpp>` and
  `<ql/quotes/deltavolquote.hpp>` instead.
- The overload of the `CPIBond` and `CPIBondHelper` constructors
  taking the `growthOnly` parameter; use the other overload instead.
- The methods `cumD1`, `cumD2`, `nD1` and `nD2` of the
  `BlackDeltaCalculator` class; they are internal methods and will be
  moved to the private section.
- The `BlackDeltaPremiumAdjustedSolverClass` and
  `BlackDeltaPremiumAdjustedMaxStrikeClass`; they were used in the
  implementation of `BlackDeltaCalculator` but are now obsolete.
- The `BootstrapError` class template; use a lambda instead (see
  <https://github.com/lballabio/QuantLib/pull/2263> for an example).
- The `PenaltyFunction` class; use `SimpleCostFunction` instead.
- The `Tona` index was renamed to `Tonar`; use the latter instead.


Full list of pull requests
--------------------------

All the pull requests merged in this release are listed on its release
page at <https://github.com/lballabio/QuantLib/releases/tag/v1.40-rc>.

The list of commits since the previous release is available in `ChangeLog.txt`.

