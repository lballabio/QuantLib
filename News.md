Changes for QuantLib 1.27:
==========================

QuantLib 1.27 includes 37 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/23?closed=1>.

Portability
-----------

- **Removed support:** as announced in the notes for the previous
  release, support for Visual Studio 2013 was dropped.

- **End of support:** as announced in the notes for the previous
  release, this release will be the last to avoid C++14 syntax.
  Allowing the newer (but still oldish) standard should still support
  most compilers released in the past several years.

- **Future end of support:** this release and the next will be the
  last to manage thread-local singletons via a user-provided
  `sessionId` function.  Future releases will use the built-in
  language support for thread-local variables.

- The `Real` type is now used consistently throughout the codebase,
  thanks to the Xcelerit dev team (@xcelerit-dev).  This, along with
  other changes, allows its default definition to `double` to be
  replaced with one of the available third-party AAD types.

- The test suite is now built using the header-only version of
  Boost.Test, thanks to Jonathan Sweemer (@sweemer).  This might
  simplify Boost installation for some users, since in the default
  configuration QuantLib now only needs the Boost headers.

- Replaced some Boost facilities with the corresponding C++11
  counterparts; thanks to Klaus Spanderen (@klausspanderen) and
  Jonathan Sweemer (@sweemer).

Date/time
---------

- Fixed the behavior of a couple of Australian holidays; thanks to
  Pradeep Krishnamurthy (@pradkrish) and Fredrik Gerdin Börjesson
  (@gbfredrik).

Instruments
-----------

- Added the Turnbull-Wakeman engine for discrete Asian options; thanks
  to Fredrik Gerdin Börjesson (@gbfredrik) for the main engine code
  and to Jack Gillett (@jackgillett101) for the Greeks.

- Added more validation to barrier options; thanks to Jonathan Sweemer
  (@sweemer).

Models
------

- Fixed the start date of the underlying swap in swaption calibration
  helpers; thanks to Peter Caspers (@pcaspers).

- Fixed parameter checks in SVI volatility smiles; thanks to Fredrik
  Gerdin Börjesson (@gbfredrik).

Patterns
--------

- Avoid possible iterator invalidation while notifying observers;
  thanks to Klaus Spanderen (@klausspanderen).

Deprecated features
-------------------

- **Removed** the `--enable-disposable` and `--enable-std-unique-ptr`
  configure switches.

- **Removed** features deprecated in version 1.22:
  - the unused `AmericanCondition` and `FDAmericanCondition` classes;
  - the old-style FD shout and dividend shout engines;
  - the unused `OneFactorOperator` class;
  - the `io::to_integer` function;
  - the `ArrayProxy` and `MatrixProxy` classes.

- Deprecated the `QL_NOEXCEPT` and `QL_CONSTEXPR` macros.

- Deprecated the `QL_NULL_INTEGER` and `QL_NULL_REAL` macros.

- Deprecated some unused parts of the old-style FD framework:
  - the `PdeShortRate` class;
  - the `ShoutCondition` and `FDShoutCondition` classes;
  - the `FDDividendEngineBase`, `FDDividendEngineMerton73`,
    `FDDividendEngineShiftScale` and `FDDividendEngine` classes;
  - the `FDStepConditionEngine` and `FDEngineAdapter` classes.

- Deprecated a number of function objects in the
  `ql/math/functional.hpp` header.

- Deprecated the unused `MultiCurveSensitivities` class.

- Deprecated the unused `inner_product` function.


**Thanks go also** to Ryan Russell (@ryanrussell) for documentation fixes.
