Changes for QuantLib 1.42
=========================

Removals and deprecations
-------------------------

Features deprecated in release 1.37 were removed in this release; see <https://github.com/lballabio/QuantLib/pull/2424> for a full list.

A number of features were deprecated in this release and will be removed in a future release (probably release 1.47):

- The overloads of `CashFlows::npv`, `BondFunctions::cleanPrice` and `BondFunctions::dirtyPrice` taking a z-spread and a day-count convention; use the overload without a day counter.
- The overloads of `CashFlows::zSpread` and `BondFunctions::zSpread` taking a day-count convention; use the overload without a day counter.
- The `DefaultLogCubic` and `LogMixedLinearCubic` interpolators; use `KrugerLog` and `KrugerLogMixedLinearCubic` instead.
- The `NeumannBC`, `DirichletBC`, `CrankNicolson`, `DMinus`, `DPlus`, `DPlusDMinus`, `DZero`, `ExplicitEuler`, `ImplicitEuler` and `MixedScheme` classes; use the new finite-difference framework instead.
- The `ForwardRateStructure` adapter class; inherit from `ZeroYieldStructure` instead (you should implement `zeroYieldImpl` anyway).
- The `ql/experimental/volatility/zabr.hpp` header; include `<ql/termstructures/volatility/zabr.hpp>` instead.
- The `ql/experimental/volatility/zabrinterpolatedsmilesection.hpp` header; include `<ql/termstructures/volatility/zabrinterpolatedsmilesection.hpp>` instead.
- The `ql/experimental/volatility/zabrinterpolation.hpp` header; include `<ql/math/interpolations/zabrinterpolation.hpp>` instead.
- The `ql/experimental/volatility/zabrsmilesection.hpp` header; include `<ql/termstructures/volatility/zabrsmilesection.hpp>` instead.
- The now empty `ql/experimental/exoticoptions/kirkspreadoptionengine.hpp`, `ql/experimental/exoticoptions/spreadoption.hpp`, `ql/grid.hpp`, `ql/math/transformedgrid.hpp`, `ql/methods/finitedifferences/bsmoperator.hpp`, `ql/methods/finitedifferences/pde.hpp` and `ql/methods/finitedifferences/pdebsm.hpp` headers.


Full list of pull requests
--------------------------

All the pull requests merged in this release are listed on its release page at <https://github.com/lballabio/QuantLib/releases/tag/v1.42>.

The list of commits since the previous release is available in `ChangeLog.txt`.

