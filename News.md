Changes for QuantLib 1.32:
==========================

QuantLib 1.32 includes 30 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/29?closed=1>.


Portability
-----------

- **Possibly breaking change:** the protected `evaluationDate_` data
  member of the `SwaptionVolatilityDiscrete` class was renamed to
  `cachedReferenceDate_`.

- **Future end of support:** we're targeting the future release 1.35
  as the last to support Visual C++ 2015, g++ up to version 6.x, and
  clang up to version 4; support for those compilers will be dropped
  in release 1.36, about one year from now.  From that point onwards,
  this will allows us to enable the use of C++17 in the code base.

- **Future end of support:** at the same time as the above, we'll also
  remove the configure switch that allows to use `boost::tuple`,
  `boost::function` and `boost::bind` instead of their `std`
  counterparts; starting from this release, the `std` classes are
  already the default.

- Reorganized the CMake presets; thanks to the XAD team
  (@auto-differentiation-dev).


Cash flows
----------

- All cash flows are now lazy; thanks to Peter Caspers (@pcaspers).


Instruments
-----------

- Overnight-indexed swaps can now have different schedules and
  nominals on the two legs; thanks to Tom Anderson
  (@tomwhoiscontrary).

- Margrabe options, compound options and chooser options were moved
  from experimental to core (@lballabio).

- Introduced common base class `FixedVsFloatingSwap` for vanilla swap
  and overnight-indexed swaps; this will be used in the future to help
  a few existing swap engines support OIS (@lballabio).

- Added optional `redemptions` argument to amortizing bond
  constructors.  This allows them to be used for pools of loans where a
  certain proportion of the underlying loans are subject to defaults
  and losses.  Thanks to Gyan Sinha (@gyansinha).

- It is now possible to manually prune the notification tree for swaps
  and bonds if one knows that the cashflows won't change pricer;
  thanks to Peter Caspers (@pcaspers).


Models
------

- Fixed the algorithm to add instruments to the calibration set of the
  Markov model; thanks to Peter Caspers (@pcaspers) for the fix and
  Giuseppe Trapani (@lePidduN7) for the heads-up.


Term structures
---------------

- Time-to-date conversion in some swaption volatility classes could
  return the wrong date before the first exercise date; this is now
  fixed, thanks to Peter Caspers (@pcaspers).

- It's now possible to specify the maximum number of iteration for the
  solver inside a bootstrapped term structure; thanks to Jonathan
  Sweemer (@sweemer) for the change and Daniel Ángeles Ortiz (@Danie8)
  for the heads-up.

- Reduced the number of notifications for bootstrap helpers; thanks to
  Peter Caspers (@pcaspers).


Random numbers
--------------

- Added the xoshiro265** random-number generator; thanks to Ralf
  Konrad (@ralfkonrad).  It is faster than the Mersenne Twister and
  might be used as default in the future.


Examples
--------

- The code of the examples has been modernized a bit; thanks to
  Jonathan Sweemer (@sweemer).


Patterns
--------

- Avoided a possible crash when using observables in a multi-threaded
  setting; thanks to Peter Caspers (@pcaspers).


Deprecated features
-------------------

- **Removed** features deprecated in version 1.27:
  - The `QL_NULL_INTEGER`, `QL_NULL_REAL`, `QL_NOEXCEPT`,
    `QL_CONSTEXPR` and `QL_USE_STD_UNIQUE_PTR` macros.
  - The `MultiCurveSensitivities` class.
  - The `constant`, `identity`, `square`, `cube`, `fourth_power`,
    `add`, `subtract`, `subtract_from`, `multiply_by`, `divide`,
    `divide_by`, `less_than`, `greater_than`, `greater_or_equal_to`,
    `not_zero`, `not_null`, `everywhere`, `nowhere`, `equal_within`,
    `clipped_function`, `clip`, `composed_function`, `compose`,
    `binary_compose3_function` and `compose3` functors.
  - The `PdeShortRate`, `ShoutCondition`, `FDShoutCondition`,
    `FDStepConditionEngine` and `FDEngineAdapter` classes from the old
    finite-differences framework.
  - The `dsd::inner_product` function.
  - The `FDDividendEngineBase`, `FDDividendEngineMerton73`,
    `FDDividendEngineShiftScale` and `FDDividendEngine` pricing
    engines.
  - The empty headers `ql/auto_ptr.hpp`, `ql/math/initializers.hpp`,
    `ql/methods/finitedifferences/americancondition.hpp`,
    `ql/methods/finitedifferences/onefactoroperator.hpp`,
    `ql/pricingengines/vanilla/fddividendshoutengine.hpp`,
    `ql/pricingengines/vanilla/fdshoutengine.hpp` and
    `ql/utilities/disposable.hpp`.

- Deprecated the `StandardFiniteDifferenceModel`,
  `StandardSystemFiniteDifferenceModel` and `StandardStepCondition`
  typedefs; define your own typedefs if needed.

- Deprecated the `FDVanillaEngine`, `FDMultiPeriodEngine`,
  `StepConditionSet`, `ParallelEvolverTraits`, `ParallelEvolver` and
  `SampledCurve`classes and the `BSMTermOperator` and
  `SampledCurveSet` typedefs; use the new finite-differences framework
  instead.

- Deprecated the `QL_NULL_FUNCTION` macro; to check if a function is
  empty, use it in a bool context instead.

- Deprecated the now empty headers
  `ql/experimental/exoticoptions/margrabeoption.hpp`,
  `ql/experimental/exoticoptions/analyticcomplexchooserengine.hpp`,
  `ql/experimental/exoticoptions/analyticeuropeanmargrabeengine.hpp`,
  `ql/experimental/exoticoptions/analyticcompoundoptionengine.hpp`,
  `ql/experimental/exoticoptions/simplechooseroption.hpp`,
  `ql/experimental/exoticoptions/compoundoption.hpp`,
  `ql/experimental/exoticoptions/analyticamericanmargrabeengine.hpp`,
  `ql/experimental/exoticoptions/analyticsimplechooserengine.hpp`,
  `ql/experimental/exoticoptions/complexchooseroption.hpp`,
  `ql/experimental/termstructures/multicurvesensitivities.hpp`,
  `ql/methods/finitedifferences/shoutcondition.hpp`,
  `ql/methods/finitedifferences/pdeshortrate.hpp`,
  `ql/pricingengines/vanilla/fddividendengine.hpp`,
  `ql/pricingengines/vanilla/fdstepconditionengine.hpp`,
  `ql/pricingengines/vanilla/fdconditions.hpp` and
  `ql/models/marketmodels/duffsdeviceinnerproduct.hpp`.


**Thanks go also** to Jonathan Sweemer (@sweemer), Ralf Konrad
(@ralfkonrad), Klaus Spanderen (@klausspanderen), Peter Caspers
(@pcaspers), Tom Anderson (@tomwhoiscontrary) and Fredrik Gerdin
Börjesson (@gbfredrik) for a number of smaller fixes and improvements.
