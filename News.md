Changes for QuantLib 1.41
=========================

Removals and deprecations
-------------------------

Features deprecated in release 1.36 were removed in this release; see
<https://github.com/lballabio/QuantLib/pull/2348> for a full list.

A number of features were deprecated in this release and will be
removed in a future release:

- The `ZeroInflationTermStructure::zeroRate` and
  `YoYInflationTermStructure::yoyRate` overloads taking an observation
  lag; if needed, use the other overloads instead (but you should go
  through the corresponding indexes anyway).
- The constructors of
  `InterpolatedPiecewiseForwardSpreadedTermStructure`,
  `InterpolatedPiecewiseZeroSpreadedTermStructure` and
  `ZeroSpreadedTermStructure` taking a day counter; use another
  constructor instead.
- The `ContinuousArithmeticAsianLevyEngine` constructor taking a start
  date; use the other constructor and pass the start date to the
  option instead.
- The `ql/functional.hpp` header; use `#include <functional>` instead.
- The `ql/tuple.hpp` header; use `#include <tuple>` instead.
- The now empty `ql/experimental/averageois/arithmeticaverageois.hpp`,
  `ql/experimental/averageois/arithmeticoisratehelper.hpp`,
  `ql/experimental/averageois/makearithmeticaverageois.hpp`,
  `ql/experimental/risk/creditriskplus.hpp` and
  `ql/experimental/risk/sensitivityanalysis.hpp` headers.


Full list of pull requests
--------------------------

All the pull requests merged in this release are listed on its release
page at <https://github.com/lballabio/QuantLib/releases/tag/v1.41>.

The list of commits since the previous release is available in `ChangeLog.txt`.

